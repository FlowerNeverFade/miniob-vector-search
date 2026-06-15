/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "sql/operator/sort_physical_operator.h"

#include "common/lang/algorithm.h"

SortPhysicalOperator::SortPhysicalOperator(vector<unique_ptr<Expression>> &&expressions, vector<bool> &&asc)
    : expressions_(std::move(expressions)), asc_(std::move(asc))
{}

RC SortPhysicalOperator::open(Trx *trx)
{
  if (children_.size() != 1) {
    return RC::INTERNAL;
  }

  RC rc = children_[0]->open(trx);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  while ((rc = children_[0]->next()) == RC::SUCCESS) {
    Tuple *tuple = children_[0]->current_tuple();
    SortItem item;
    rc = ValueListTuple::make(*tuple, item.tuple);
    if (rc != RC::SUCCESS) {
      return rc;
    }

    for (const auto &expr : expressions_) {
      Value key;
      rc = expr->get_value(item.tuple, key);
      if (rc != RC::SUCCESS) {
        return rc;
      }
      item.keys.push_back(key);
    }
    items_.push_back(item);
  }

  if (rc != RC::RECORD_EOF) {
    return rc;
  }

  std::sort(items_.begin(), items_.end(), [this](const SortItem &left, const SortItem &right) {
    for (size_t i = 0; i < expressions_.size(); i++) {
      const int cmp = left.keys[i].compare(right.keys[i]);
      if (cmp != 0) {
        const bool asc = i < asc_.size() ? asc_[i] : true;
        return asc ? cmp < 0 : cmp > 0;
      }
    }
    return false;
  });

  pos_ = 0;
  return RC::SUCCESS;
}

RC SortPhysicalOperator::next()
{
  if (pos_ >= items_.size()) {
    return RC::RECORD_EOF;
  }
  pos_++;
  return RC::SUCCESS;
}

RC SortPhysicalOperator::close()
{
  items_.clear();
  pos_ = 0;
  if (!children_.empty()) {
    return children_[0]->close();
  }
  return RC::SUCCESS;
}

Tuple *SortPhysicalOperator::current_tuple()
{
  if (pos_ == 0 || pos_ > items_.size()) {
    return nullptr;
  }
  return &items_[pos_ - 1].tuple;
}

RC SortPhysicalOperator::tuple_schema(TupleSchema &schema) const
{
  if (children_.empty()) {
    return RC::INTERNAL;
  }
  return children_[0]->tuple_schema(schema);
}

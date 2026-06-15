/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "sql/operator/vector_index_scan_physical_operator.h"

#include "storage/index/ivfflat_index.h"
#include "storage/table/table.h"

VectorIndexScanPhysicalOperator::VectorIndexScanPhysicalOperator(
    Table *table, IvfflatIndex *index, vector<float> &&query_vector, int limit, bool asc)
    : table_(table), index_(index), query_vector_(std::move(query_vector)), limit_(limit), asc_(asc)
{}

RC VectorIndexScanPhysicalOperator::open(Trx *trx)
{
  (void)trx;
  if (table_ == nullptr || index_ == nullptr || limit_ < 0) {
    return RC::INVALID_ARGUMENT;
  }

  rids_ = index_->ann_search(query_vector_, static_cast<size_t>(limit_), asc_);
  pos_  = 0;
  tuple_.set_schema(table_, table_->table_meta().field_metas());
  return RC::SUCCESS;
}

RC VectorIndexScanPhysicalOperator::next()
{
  if (pos_ >= rids_.size()) {
    return RC::RECORD_EOF;
  }

  RC rc = table_->get_record(rids_[pos_], current_record_);
  if (OB_FAIL(rc)) {
    return rc;
  }
  pos_++;
  tuple_.set_record(&current_record_);
  return RC::SUCCESS;
}

RC VectorIndexScanPhysicalOperator::close()
{
  rids_.clear();
  pos_ = 0;
  return RC::SUCCESS;
}

Tuple *VectorIndexScanPhysicalOperator::current_tuple()
{
  tuple_.set_record(&current_record_);
  return &tuple_;
}

string VectorIndexScanPhysicalOperator::param() const
{
  return string(index_->index_meta().name()) + " ON " + table_->name();
}

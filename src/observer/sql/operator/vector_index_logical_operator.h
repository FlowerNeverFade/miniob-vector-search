/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#pragma once

#include "common/lang/vector.h"
#include "common/lang/utility.h"
#include "common/types.h"
#include "sql/operator/logical_operator.h"

class Index;
class Table;

class VectorIndexLogicalOperator : public LogicalOperator
{
public:
  VectorIndexLogicalOperator(Table *table, Index *index, vector<float> &&query_vector, int limit, bool asc)
      : table_(table), index_(index), query_vector_(std::move(query_vector)), limit_(limit), asc_(asc)
  {}

  LogicalOperatorType type() const override { return LogicalOperatorType::VECTOR_INDEX_SCAN; }

  Table *table() const { return table_; }
  Index *index() const { return index_; }
  const vector<float> &query_vector() const { return query_vector_; }
  int limit() const { return limit_; }
  bool asc() const { return asc_; }

private:
  Table        *table_ = nullptr;
  Index        *index_ = nullptr;
  vector<float> query_vector_;
  int           limit_ = -1;
  bool          asc_ = true;
};

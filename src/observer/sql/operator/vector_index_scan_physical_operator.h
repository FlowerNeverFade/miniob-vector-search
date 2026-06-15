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
#include "sql/expr/tuple.h"
#include "sql/operator/physical_operator.h"
#include "storage/record/record_manager.h"

class IvfflatIndex;
class Table;

class VectorIndexScanPhysicalOperator : public PhysicalOperator
{
public:
  VectorIndexScanPhysicalOperator(Table *table, IvfflatIndex *index, vector<float> &&query_vector, int limit, bool asc);

  PhysicalOperatorType type() const override { return PhysicalOperatorType::VECTOR_INDEX_SCAN; }
  string param() const override;

  RC open(Trx *trx) override;
  RC next() override;
  RC close() override;

  Tuple *current_tuple() override;

private:
  Table        *table_ = nullptr;
  IvfflatIndex *index_ = nullptr;
  vector<float> query_vector_;
  int           limit_ = -1;
  bool          asc_ = true;
  vector<RID>   rids_;
  size_t        pos_ = 0;
  Record        current_record_;
  RowTuple      tuple_;
};

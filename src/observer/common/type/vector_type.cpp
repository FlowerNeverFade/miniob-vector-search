/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "common/type/vector_type.h"

#include <cstdint>

#include "common/lang/sstream.h"
#include "common/lang/string.h"
#include "common/log/log.h"
#include "common/value.h"

int VectorType::compare(const Value &left, const Value &right) const
{
  if (left.attr_type() != AttrType::VECTORS || right.attr_type() != AttrType::VECTORS ||
      left.length() != right.length()) {
    return INT32_MAX;
  }

  const int dim = left.length() / static_cast<int>(sizeof(float));
  const float *left_values = reinterpret_cast<const float *>(left.data());
  const float *right_values = reinterpret_cast<const float *>(right.data());
  for (int i = 0; i < dim; i++) {
    if (left_values[i] < right_values[i]) {
      return -1;
    }
    if (left_values[i] > right_values[i]) {
      return 1;
    }
  }
  return 0;
}

RC VectorType::to_string(const Value &val, string &result) const
{
  if (val.attr_type() != AttrType::VECTORS || val.length() % static_cast<int>(sizeof(float)) != 0) {
    return RC::INVALID_ARGUMENT;
  }

  stringstream ss;
  ss << "[";
  const int dim = val.length() / static_cast<int>(sizeof(float));
  const float *values = reinterpret_cast<const float *>(val.data());
  for (int i = 0; i < dim; i++) {
    if (i != 0) {
      ss << ",";
    }
    ss << common::double_to_str(values[i]);
  }
  ss << "]";
  result = ss.str();
  return RC::SUCCESS;
}

RC VectorType::set_value_from_str(Value &val, const string &data) const
{
  return val.set_vector_from_string(data.c_str(), static_cast<int>(data.size()));
}

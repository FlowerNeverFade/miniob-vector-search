/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Wangyunlai.wyl on 2021/5/18.
//

#include "storage/index/index_meta.h"
#include "common/lang/string.h"
#include "common/log/log.h"
#include "storage/field/field_meta.h"
#include "storage/table/table_meta.h"
#include "json/json.h"

const static Json::StaticString FIELD_NAME("name");
const static Json::StaticString FIELD_FIELD_NAME("field_name");
const static Json::StaticString FIELD_IS_VECTOR("is_vector");
const static Json::StaticString FIELD_VECTOR_TYPE("type");
const static Json::StaticString FIELD_DISTANCE("distance");
const static Json::StaticString FIELD_LISTS("lists");
const static Json::StaticString FIELD_PROBES("probes");

RC IndexMeta::init(const char *name, const FieldMeta &field)
{
  return init(name, field, false, "ivfflat", "EUCLIDEAN", 245, 5);
}

RC IndexMeta::init(const char *name,
                   const FieldMeta &field,
                   bool is_vector,
                   const char *vector_type,
                   const char *distance,
                   int lists,
                   int probes)
{
  if (common::is_blank(name)) {
    LOG_ERROR("Failed to init index, name is empty.");
    return RC::INVALID_ARGUMENT;
  }

  if (is_vector && (common::is_blank(vector_type) || common::is_blank(distance) || lists <= 0 || probes <= 0)) {
    LOG_ERROR("Failed to init vector index, options are invalid. name=%s, type=%s, distance=%s, lists=%d, probes=%d",
        name, vector_type, distance, lists, probes);
    return RC::INVALID_ARGUMENT;
  }

  name_        = name;
  field_       = field.name();
  is_vector_   = is_vector;
  vector_type_ = common::is_blank(vector_type) ? "ivfflat" : vector_type;
  distance_    = common::is_blank(distance) ? "EUCLIDEAN" : distance;
  lists_       = lists;
  probes_      = probes;
  return RC::SUCCESS;
}

void IndexMeta::to_json(Json::Value &json_value) const
{
  json_value[FIELD_NAME]       = name_;
  json_value[FIELD_FIELD_NAME] = field_;
  json_value[FIELD_IS_VECTOR]  = is_vector_;
  if (is_vector_) {
    json_value[FIELD_VECTOR_TYPE] = vector_type_;
    json_value[FIELD_DISTANCE]    = distance_;
    json_value[FIELD_LISTS]       = lists_;
    json_value[FIELD_PROBES]      = probes_;
  }
}

RC IndexMeta::from_json(const TableMeta &table, const Json::Value &json_value, IndexMeta &index)
{
  const Json::Value &name_value  = json_value[FIELD_NAME];
  const Json::Value &field_value = json_value[FIELD_FIELD_NAME];
  if (!name_value.isString()) {
    LOG_ERROR("Index name is not a string. json value=%s", name_value.toStyledString().c_str());
    return RC::INTERNAL;
  }

  if (!field_value.isString()) {
    LOG_ERROR("Field name of index [%s] is not a string. json value=%s",
        name_value.asCString(), field_value.toStyledString().c_str());
    return RC::INTERNAL;
  }

  const FieldMeta *field = table.field(field_value.asCString());
  if (nullptr == field) {
    LOG_ERROR("Deserialize index [%s]: no such field: %s", name_value.asCString(), field_value.asCString());
    return RC::SCHEMA_FIELD_MISSING;
  }

  bool   is_vector   = false;
  string vector_type = "ivfflat";
  string distance    = "EUCLIDEAN";
  int    lists       = 245;
  int    probes      = 5;

  const Json::Value &is_vector_value = json_value[FIELD_IS_VECTOR];
  if (!is_vector_value.isNull()) {
    if (!is_vector_value.isBool()) {
      LOG_ERROR("Index is_vector is not a bool. json value=%s", is_vector_value.toStyledString().c_str());
      return RC::INTERNAL;
    }
    is_vector = is_vector_value.asBool();
  }

  if (is_vector) {
    const Json::Value &type_value     = json_value[FIELD_VECTOR_TYPE];
    const Json::Value &distance_value = json_value[FIELD_DISTANCE];
    const Json::Value &lists_value    = json_value[FIELD_LISTS];
    const Json::Value &probes_value   = json_value[FIELD_PROBES];
    if (!type_value.isString() || !distance_value.isString() || !lists_value.isInt() || !probes_value.isInt()) {
      LOG_ERROR("Vector index options are invalid. json value=%s", json_value.toStyledString().c_str());
      return RC::INTERNAL;
    }
    vector_type = type_value.asString();
    distance    = distance_value.asString();
    lists       = lists_value.asInt();
    probes      = probes_value.asInt();
  }

  return index.init(name_value.asCString(), *field, is_vector, vector_type.c_str(), distance.c_str(), lists, probes);
}

const char *IndexMeta::name() const { return name_.c_str(); }

const char *IndexMeta::field() const { return field_.c_str(); }

void IndexMeta::desc(ostream &os) const
{
  os << "index name=" << name_ << ", field=" << field_;
  if (is_vector_) {
    os << ", vector type=" << vector_type_ << ", distance=" << distance_ << ", lists=" << lists_
       << ", probes=" << probes_;
  }
}

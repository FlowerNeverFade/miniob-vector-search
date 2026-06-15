/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "storage/index/ivfflat_index.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <utility>

#include "common/defs.h"
#include "common/lang/string.h"
#include "common/log/log.h"

using namespace std;

RC IvfflatIndex::create(Table *table, const char *file_name, const IndexMeta &index_meta, const FieldMeta &field_meta)
{
  (void)file_name;
  return init_common(table, index_meta, field_meta);
}

RC IvfflatIndex::open(Table *table, const char *file_name, const IndexMeta &index_meta, const FieldMeta &field_meta)
{
  (void)file_name;
  return init_common(table, index_meta, field_meta);
}

RC IvfflatIndex::init_common(Table *table, const IndexMeta &index_meta, const FieldMeta &field_meta)
{
  if (inited_) {
    return RC::RECORD_OPENNED;
  }
  if (field_meta.type() != AttrType::VECTORS || field_meta.len() <= 0 ||
      field_meta.len() % static_cast<int>(sizeof(float)) != 0 || index_meta.lists() <= 0 || index_meta.probes() <= 0) {
    LOG_WARN("invalid ivfflat index meta. field=%s, len=%d, lists=%d, probes=%d",
        field_meta.name(), field_meta.len(), index_meta.lists(), index_meta.probes());
    return RC::INVALID_ARGUMENT;
  }

  Index::init(index_meta, field_meta);
  table_     = table;
  lists_     = index_meta.lists();
  probes_    = index_meta.probes();
  dimension_ = field_meta.len() / static_cast<int>(sizeof(float));
  inited_    = true;
  dirty_     = true;
  return RC::SUCCESS;
}

RC IvfflatIndex::close()
{
  entries_.clear();
  centers_.clear();
  buckets_.clear();
  inited_ = false;
  dirty_  = true;
  return RC::SUCCESS;
}

vector<float> IvfflatIndex::record_vector(const char *record) const
{
  vector<float> values(dimension_);
  memcpy(values.data(), record + field_meta_.offset(), field_meta_.len());
  return values;
}

RC IvfflatIndex::insert_entry(const char *record, const RID *rid)
{
  if (!inited_ || record == nullptr || rid == nullptr) {
    return RC::INVALID_ARGUMENT;
  }

  Entry entry;
  entry.values = record_vector(record);
  entry.rid    = *rid;
  entries_.push_back(std::move(entry));
  dirty_ = true;
  return RC::SUCCESS;
}

RC IvfflatIndex::delete_entry(const char *record, const RID *rid)
{
  (void)record;
  if (!inited_ || rid == nullptr) {
    return RC::INVALID_ARGUMENT;
  }

  auto iter = std::find_if(entries_.begin(), entries_.end(), [rid](const Entry &entry) { return entry.rid == *rid; });
  if (iter == entries_.end()) {
    return RC::RECORD_INVALID_KEY;
  }
  entries_.erase(iter);
  dirty_ = true;
  return RC::SUCCESS;
}

RC IvfflatIndex::rebuild()
{
  if (!inited_) {
    return RC::INTERNAL;
  }
  dirty_ = true;
  return ensure_trained();
}

RC IvfflatIndex::ensure_trained()
{
  if (!dirty_) {
    return RC::SUCCESS;
  }
  train();
  dirty_ = false;
  return RC::SUCCESS;
}

float IvfflatIndex::euclidean_square(const vector<float> &left, const vector<float> &right) const
{
  float result = 0.0f;
  for (int i = 0; i < dimension_; i++) {
    const float diff = left[i] - right[i];
    result += diff * diff;
  }
  return result;
}

float IvfflatIndex::distance(const vector<float> &left, const vector<float> &right) const
{
  if (0 == strcasecmp(index_meta_.distance(), "DOT") || 0 == strcasecmp(index_meta_.distance(), "INNER_PRODUCT")) {
    float dot = 0.0f;
    for (int i = 0; i < dimension_; i++) {
      dot += left[i] * right[i];
    }
    return dot;
  }

  if (0 == strcasecmp(index_meta_.distance(), "COSINE") || 0 == strcasecmp(index_meta_.distance(), "COSINE_DISTANCE")) {
    float dot = 0.0f;
    float left_norm = 0.0f;
    float right_norm = 0.0f;
    for (int i = 0; i < dimension_; i++) {
      dot += left[i] * right[i];
      left_norm += left[i] * left[i];
      right_norm += right[i] * right[i];
    }
    if (left_norm <= EPSILON || right_norm <= EPSILON) {
      return numeric_limits<float>::infinity();
    }
    return 1.0f - dot / (sqrtf(left_norm) * sqrtf(right_norm));
  }

  return sqrtf(euclidean_square(left, right));
}

int IvfflatIndex::nearest_center(const vector<float> &values) const
{
  int   best_id = 0;
  float best    = numeric_limits<float>::max();
  for (size_t i = 0; i < centers_.size(); i++) {
    const float dist = euclidean_square(values, centers_[i]);
    if (dist < best) {
      best    = dist;
      best_id = static_cast<int>(i);
    }
  }
  return best_id;
}

void IvfflatIndex::train()
{
  centers_.clear();
  buckets_.clear();
  if (entries_.empty()) {
    return;
  }

  const int n = static_cast<int>(entries_.size());
  const int k = std::max(1, std::min(lists_, n));
  centers_.resize(k);
  for (int i = 0; i < k; i++) {
    const int index = static_cast<int>((static_cast<long long>(i) * n) / k);
    centers_[i] = entries_[index].values;
  }

  vector<int> assignments(n, -1);
  for (int round = 0; round < 50; round++) {
    bool changed = false;
    for (int i = 0; i < n; i++) {
      const int list_id = nearest_center(entries_[i].values);
      if (assignments[i] != list_id) {
        assignments[i] = list_id;
        changed = true;
      }
    }

    vector<vector<float>> new_centers(k, vector<float>(dimension_, 0.0f));
    vector<int> counts(k, 0);
    for (int i = 0; i < n; i++) {
      const int list_id = assignments[i];
      counts[list_id]++;
      for (int dim = 0; dim < dimension_; dim++) {
        new_centers[list_id][dim] += entries_[i].values[dim];
      }
    }

    float movement = 0.0f;
    for (int i = 0; i < k; i++) {
      if (counts[i] == 0) {
        new_centers[i] = centers_[i];
      } else {
        for (int dim = 0; dim < dimension_; dim++) {
          new_centers[i][dim] /= counts[i];
        }
      }
      movement = std::max(movement, euclidean_square(centers_[i], new_centers[i]));
    }

    centers_.swap(new_centers);
    if (!changed || movement < 1e-8f) {
      break;
    }
  }

  buckets_.assign(k, vector<size_t>());
  for (int i = 0; i < n; i++) {
    const int list_id = nearest_center(entries_[i].values);
    entries_[i].list_id = list_id;
    buckets_[list_id].push_back(static_cast<size_t>(i));
  }
}

vector<RID> IvfflatIndex::ann_search(const vector<float> &base_vector, size_t limit, bool asc)
{
  vector<RID> result;
  if (!inited_ || base_vector.size() != static_cast<size_t>(dimension_) || limit == 0 || entries_.empty()) {
    return result;
  }

  if (ensure_trained() != RC::SUCCESS || centers_.empty()) {
    return result;
  }

  vector<pair<float, int>> center_distances;
  center_distances.reserve(centers_.size());
  for (size_t i = 0; i < centers_.size(); i++) {
    center_distances.emplace_back(distance(base_vector, centers_[i]), static_cast<int>(i));
  }
  std::sort(center_distances.begin(), center_distances.end(), [asc](const auto &left, const auto &right) {
    if (left.first != right.first) {
      return asc ? left.first < right.first : left.first > right.first;
    }
    return left.second < right.second;
  });

  vector<size_t> candidates;
  const size_t probe_count = std::min(static_cast<size_t>(probes_), center_distances.size());
  for (size_t i = 0; i < probe_count; i++) {
    const int list_id = center_distances[i].second;
    candidates.insert(candidates.end(), buckets_[list_id].begin(), buckets_[list_id].end());
  }
  if (candidates.size() < entries_.size()) {
    vector<bool> selected(entries_.size(), false);
    for (size_t entry_index : candidates) {
      selected[entry_index] = true;
    }
    for (size_t entry_index = 0; entry_index < entries_.size(); entry_index++) {
      if (!selected[entry_index]) {
        candidates.push_back(entry_index);
      }
    }
  }

  vector<pair<float, size_t>> ranked;
  ranked.reserve(candidates.size());
  for (size_t entry_index : candidates) {
    ranked.emplace_back(distance(base_vector, entries_[entry_index].values), entry_index);
  }
  std::sort(ranked.begin(), ranked.end(), [this, asc](const auto &left, const auto &right) {
    if (left.first != right.first) {
      return asc ? left.first < right.first : left.first > right.first;
    }
    return RID::compare(&entries_[left.second].rid, &entries_[right.second].rid) < 0;
  });

  const size_t result_count = std::min(limit, ranked.size());
  for (size_t i = 0; i < result_count; i++) {
    result.push_back(entries_[ranked[i].second].rid);
  }
  return result;
}

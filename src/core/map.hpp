#pragma once

#include <unordered_map>

template <typename K, typename V>
using HashMap = std::unordered_map<K, V>;

template <typename K, typename V>
bool HasKey(const HashMap<K, V> &map, const K &key)
{
    return map.find(key) != map.end();
}
#ifndef SF_STL_TYPES_HPP
#define SF_STL_TYPES_HPP

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

///
/// @brief Alias for a shared pointer (a "reference").
///
template<typename T>
using Ref = std::shared_ptr<T>;

///
/// @brief Alias for a string.
///
using String = std::string;

///
/// @brief Alias for an unordered (hash) map.
///
template<typename TKey, typename TVal, typename THash = std::hash<TKey>>
using Map = std::unordered_map<TKey, TVal, THash>;

///
/// @brief Alias for an ordered (tree) map.
///
template<typename TKey, typename TVal, typename TCompare = std::less<TKey>>
using OrderedMap = std::map<TKey, TVal, TCompare>;

///
/// @brief Alias for a hash set.
///
template<typename T, typename THash = std::hash<T>>
using Set = std::unordered_set<T, THash>;

///
/// @brief Alias for a vector.
///
template<typename T>
using Vec = std::vector<T>;

#endif

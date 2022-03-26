#ifndef SF_STL_TYPES_HPP
#define SF_STL_TYPES_HPP

#include <map>
#include <memory>
#include <set>
#include <string>
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
/// @brief Alias for a map.
///
template<typename TKey, typename TVal, typename TCompare = std::less<TKey>>
using Map = std::map<TKey, TVal, TCompare>;

///
/// @brief Alias for a set.
///
template<typename T, typename TCompare = std::less<T>>
using Set = std::set<T, TCompare>;

///
/// @brief Alias for a vector.
///
template<typename T>
using Vec = std::vector<T>;

#endif

////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
///                             ---------------
/// @file  sf/config/StlTypes.hpp
/// @brief Aliases for STL types used throughout Surefire.
////////////////////////////////////////////////////////////////////////////////

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

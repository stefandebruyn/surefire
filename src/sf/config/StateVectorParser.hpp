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
/// @file  sf/config/StateVectorParser.hpp
/// @brief Parser for state vector configs.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_STATE_VECTOR_PARSER_HPP
#define SF_STATE_VECTOR_PARSER_HPP

#include <istream>

#include "sf/config/ErrorInfo.hpp"
#include "sf/config/StlTypes.hpp"
#include "sf/config/TokenIterator.hpp"
#include "sf/core/StateVector.hpp"

///
/// @brief Parse of a state vector.
///
/// @see StateVectorParser
///
class StateVectorParse final
{
public:

    ///
    /// @brief Parse of a state vector element.
    ///
    struct ElementParse final
    {
        Token tokType; ///< Element type.
        Token tokName; ///< Element name.
    };

    ///
    /// @brief Parse of a region.
    ///
    struct RegionParse final
    {
        Token tokName;                             ///< Region section token.
        String plainName;                          ///< Plain region name.
        Vec<StateVectorParse::ElementParse> elems; ///< Elements in region.
    };

    ///
    /// @brief State vector config options.
    ///
    struct Options final
    {
        bool lock; ///< If state vector is thread-safe.
    };

    ///
    /// @brief State vector regions.
    ///
    Vec<StateVectorParse::RegionParse> regions;

    ///
    /// @brief State vector config options.
    ///
    Options opts;

private:

    friend class StateVectorParser;

    ///
    /// @brief Constructor.
    ///
    /// @param[in] kRegions  State vector regions.
    /// @param[in] kOpts     State vector config options.
    ///
    StateVectorParse(Vec<StateVectorParse::RegionParse>& kRegions,
                     const StateVectorParse::Options& kOpts);
};

///
/// @brief State vector parser.
///
class StateVectorParser final
{
public:

    ///
    /// @brief When passed as kRegions to parse(), all regions in the state
    /// vector config will be parsed.
    ///
    static const Vec<String> ALL_REGIONS;

    ///
    /// @brief Parser entry point.
    ///
    /// @param[in]  kToks     Tokens to parse.
    /// @param[out] kParse    On success, points to state vector parse.
    /// @param[out] kErr      On error, if non-null, contains error info.
    /// @param[in]  kRegions  Names of regions to parse, or ALL_REGIONS if all
    ///                       regions in the config should be parsed.
    ///
    /// @retval SUCCESS          Successfully parsed state vector.
    /// @retval E_SVP_TOK        Unexpected token.
    /// @retval E_SVP_RGN        kRegions names a region not in the config.
    /// @retval E_SVP_ELEM_TYPE  Invalid element type.
    /// @retval E_SVP_ELEM_NAME  Expected element name.
    /// @retval E_SVP_OPT        Invalid option.
    ///
    static Result parse(const Vec<Token>& kToks,
                        Ref<const StateVectorParse>& kParse,
                        ErrorInfo* const kErr,
                        const Vec<String> kRegions = ALL_REGIONS);

    StateVectorParser() = delete;

private:

    ///
    /// @brief Parses a region.
    ///
    /// @param[in]  kIt      Token iterator positioned at section token.
    /// @param[out] kRegion  On success, contains region parse.
    /// @param[out] kErr     On error, if non-null, contains error info.
    ///
    /// @returns See StateVectorParser::parse().
    ///
    static Result parseRegion(TokenIterator& kIt,
                              StateVectorParse::RegionParse& kRegion,
                              ErrorInfo* const kErr);

    ///
    /// @brief Parses the options section.
    ///
    /// @param[in]  kIt      Token iterator positioned at section token.
    /// @param[out] kRegion  On success, contains options parse.
    /// @param[out] kErr     On error, if non-null, contains error info.
    ///
    /// @returns See StateVectorParser::parse().
    ///
    static Result parseOptions(TokenIterator& kIt,
                               StateVectorParse::Options& kOpts,
                               ErrorInfo* const kErr);
};

#endif

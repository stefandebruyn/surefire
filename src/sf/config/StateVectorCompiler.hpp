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
/// @file  sf/config/StateVectorCompiler.hpp
/// @brief State vector compiler.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_STATE_VECTOR_COMPILER_HPP
#define SF_STATE_VECTOR_COMPILER_HPP

#include <istream>

#include "sf/config/StateVectorParser.hpp"
#include "sf/pal/Spinlock.hpp"

///
/// @brief Compiled state vector.
///
class StateVectorAssembly final
{
public:

    ///
    /// @brief Gets the underlying StateVector object.
    ///
    StateVector& get() const;

    ///
    /// @brief Gets the config used to configure the state vector.
    ///
    /// @remark This is mostly for testing purposes and should not be accessed
    /// in production.
    ///
    /// @returns State vector config.
    ///
    StateVector::Config config() const;

    ///
    /// @brief Gets the parse used to compile the state vector.
    ///
    /// @returns State vector parse.
    ///
    Ref<const StateVectorParse> parse() const;

private:

    friend class StateVectorCompiler;

    ///
    /// @brief Set of data that represents the state vector.
    ///
    struct Workspace final
    {
        ///
        /// @brief Element configs.
        ///
        Ref<Vec<StateVector::ElementConfig>> elemConfigs;

        ///
        /// @brief Region configs.
        ///
        Ref<Vec<StateVector::RegionConfig>> regionConfigs;

        ///
        /// @brief State vector backing memory.
        ///
        Ref<Vec<U8>> svBacking;

        ///
        /// @brief Strings that appear in element and region configs.
        ///
        Vec<Ref<String>> configStrings;

        ///
        /// @brief Element objects.
        ///
        Vec<Ref<IElement>> elems;

        ///
        /// @brief Region objects.
        ///
        Vec<Ref<Region>> regions;

        ///
        /// @brief State vector lock, or null if none.
        ///
        Ref<Spinlock> lock;

        ///
        /// @brief Main state vector object.
        ///
        Ref<StateVector> sv;

        ///
        /// @brief State vector config.
        ///
        StateVector::Config svConfig;

        ///
        /// @brief Parse used to compile the state vector.
        ///
        Ref<const StateVectorParse> svParse;
    };

    ///
    /// @brief Workspace.
    ///
    const StateVectorAssembly::Workspace mWs;

    ///
    /// @brief Contructor.
    ///
    /// @param[in] kWs  Workspace.
    ///
    StateVectorAssembly(const StateVectorAssembly::Workspace& kWs);
};

///
/// @brief State vector compiler.
///
class StateVectorCompiler final
{
public:

    ///
    /// @brief Compiler entry point, taking a path to the state vector config
    /// file.
    ///
    /// @param[in]  kFilePath  Path to to state vector config file.
    /// @param[out] kAsm       On success, points to compiled state vector.
    /// @param[out] kErr       On error, if non-null, contains error info.
    ///
    /// @retval SUCCESS          Successfully compiled state vector.
    /// @retval E_SMC_FILE       Failed to open state vector config file.
    /// @retval E_SVC_NULL       State vector parse is null.
    /// @retval E_SVC_RGN_DUPE   Reused region name.
    /// @retval E_SVC_ELEM_DUPE  Reused element name.
    /// @retval E_SVC_RGN_EMPTY  Region contains no elements.
    /// @retval E_SVC_ELEM_TYPE  Invalid element type.
    ///
    static Result compile(const String kFilePath,
                          Ref<const StateVectorAssembly>& kAsm,
                          ErrorInfo* const kErr);

    ///
    /// @brief Compiler entry point, taking an input stream of the state vector
    /// config.
    ///
    /// @see StateVectorCompiler::compile(String, ...)
    ///
    static Result compile(std::istream& kIs,
                          Ref<const StateVectorAssembly>& kAsm,
                          ErrorInfo* const kErr);

    ///
    /// @brief Compiler entry point, taking a state vector parse.
    ///
    /// @see StateVectorCompiler::compile(String, ...)
    ///
    static Result compile(const Ref<const StateVectorParse> kParse,
                          Ref<const StateVectorAssembly>& kAsm,
                          ErrorInfo* const kErr);

    StateVectorCompiler() = delete;

private:

    ///
    /// @brief Compiles an element in the state vector.
    ///
    /// @param[in]       kElem        Element parse to compile.
    /// @param[in]       kWs          Compiler workspace.
    /// @param[out]      kElemConfig  On success, contains element config.
    /// @param[in, out]  kBumpPtr     Address of element backing storage. On
    ///                               success, will be bumped to the address of
    ///                               the next element to be allocated.
    ///
    /// @retval SUCCESS  Always succeeds (unless an assertion fails).
    ///
    static Result allocateElement(const StateVectorParse::ElementParse& kElem,
                                  StateVectorAssembly::Workspace& kWs,
                                  StateVector::ElementConfig& kElemConfig,
                                  U8*& kBumpPtr);
};

#endif

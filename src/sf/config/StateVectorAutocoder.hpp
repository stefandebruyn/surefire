////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
///
/// Copyright (c) 2022 the Surefire authors. All rights reserved.
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
/// @file  sf/config/StateVectorAutocoder.hpp
/// @brief State vector autocoder.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_STATE_VECTOR_AUTOCODER_HPP
#define SF_STATE_VECTOR_AUTOCODER_HPP

#include "sf/config/StateVectorCompiler.hpp"

namespace Sf
{

///
/// @brief State vector autocoder.
///
namespace StateVectorAutocoder
{
    ///
    /// @brief State vector autocoder entry point.
    ///
    /// @param[in] kOs     Autocode output stream.
    /// @param[in] kName   Name of state vector (will be used for certain
    ///                    identifiers in autocode).
    /// @param[in] kSvAsm  Compiled state vector to autocode.
    ///
    Result code(std::ostream& kOs,
                const String kName,
                const Ref<const StateVectorAssembly> kSvAsm);
}

} // namespace Sf

#endif

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
////////////////////////////////////////////////////////////////////////////////

#include "sf/config/Autocode.hpp"

Autocode::Autocode(std::ostream& kOs) : mOs(kOs), mIndentLvl(0)
{
}

Autocode::~Autocode()
{
    this->flush();
}

void Autocode::increaseIndent()
{
    ++mIndentLvl;
}

void Autocode::decreaseIndent()
{
    if (mIndentLvl > 0)
    {
        --mIndentLvl;
    }
}

void Autocode::operator()()
{
    mOs << "\n";
}

void Autocode::flush()
{
    mOs.flush();
}

String& Autocode::formatStep(String& kStr)
{
    return kStr;
}

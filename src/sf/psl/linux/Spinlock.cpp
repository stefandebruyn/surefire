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

#include "sf/pal/Spinlock.hpp"

Result Spinlock::init(Spinlock& kLock)
{
    // Check that spinlock is not already initialized.
    if (kLock.mInit)
    {
        return E_SLK_REINIT;
    }

    if (pthread_spin_init(&kLock.mLock, PTHREAD_PROCESS_PRIVATE) != 0)
    {
        return E_SLK_CREATE;
    }

    kLock.mInit = true;
    return SUCCESS;
}

Spinlock::Spinlock() : mInit(false)
{
}

Spinlock::~Spinlock()
{
    (void) pthread_spin_destroy(&mLock);
}

Result Spinlock::acquire()
{
    if (!mInit)
    {
        return E_SLK_UNINIT;
    }

    if (pthread_spin_lock(&mLock) != 0)
    {
        return E_SLK_ACQ;
    }

    return SUCCESS;
}

Result Spinlock::release()
{
    if (!mInit)
    {
        return E_SLK_UNINIT;
    }

    if (pthread_spin_unlock(&mLock) != 0)
    {
        return E_SLK_REL;
    }

    return SUCCESS;
}

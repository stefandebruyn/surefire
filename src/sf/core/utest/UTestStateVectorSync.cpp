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
/// @file  sf/core/utest/UTestStateVectorSync.cpp
/// @brief Unit tests for Element and Region thread-safety.
////////////////////////////////////////////////////////////////////////////////

#include "sf/core/Element.hpp"
#include "sf/core/Region.hpp"
#include "sf/pal/Clock.hpp"
#include "sf/pal/Spinlock.hpp"
#include "sf/pal/Thread.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

// Arguments used by test threads.
struct ThreadArgs final
{
    Element<I32>* elemFoo;
    Element<I32>* elemBar;
    Region* regionFoo;
    Region* regionBar;
};

// Thread which writes 2 elements.
static Result writeElemThread(void* kArgs)
{
    const ThreadArgs* const args = reinterpret_cast<const ThreadArgs*>(kArgs);
    args->elemFoo->write(100);
    args->elemBar->write(100);
    return SUCCESS;
}

// Thread which writes 2 regions.
static Result writeRegionThread(void* kArgs)
{
    const ThreadArgs* const args = reinterpret_cast<const ThreadArgs*>(kArgs);
    I32 val = 100;
    args->regionFoo->write(&val, sizeof(val));
    args->regionBar->write(&val, sizeof(val));
    return SUCCESS;
}

// Thread which reads an element and returns its value.
static Result readElemThread(void* kArgs)
{
    const Element<I32>* const elem =
        reinterpret_cast<const Element<I32>*>(kArgs);
    return static_cast<Result>(elem->read());
}

// Thread which reads a 4-byte region as an I32 and returns its value.
static Result readRegionThread(void* kArgs)
{
    const Region* const region = reinterpret_cast<const Region*>(kArgs);
    I32 val = 0;
    (void) region->read(&val, sizeof(val));
    return static_cast<Result>(val);
}

//////////////////////////////////// Tests /////////////////////////////////////

///
/// @brief Unit tests for Element and Region thread-safety.
///
/// @note These tests require that the PSL supports real-time threads and at
/// least 2 CPU cores.
///
TEST_GROUP(StateVectorSync)
{
    void setup()
    {
        // Pin unit test thread to core 0 so that it can run parallel to the
        // thread created to contend for the lock.
        CHECK_SUCCESS(Thread::set((Thread::REALTIME_MIN_PRI + 1),
                                Thread::REALTIME,
                                0));
    }
};

///
/// @test Writing an element acquires and releases its lock.
///
TEST(StateVectorSync, ElementWriteSync)
{
    // Create and acquire spinlock.
    Spinlock lock;
    CHECK_SUCCESS(Spinlock::init(lock));
    CHECK_SUCCESS(lock.acquire());

    // Create element without lock.
    I32 foo = 0;
    Element<I32> elemFoo(foo);

    // Create element with lock.
    I32 bar = 0;
    Element<I32> elemBar(bar, &lock);

    // Create thread to write both elements.
    Thread thread;
    ThreadArgs args = {&elemFoo, &elemBar, nullptr, nullptr};
    CHECK_SUCCESS(Thread::init(writeElemThread,
                               &args,
                               Thread::REALTIME_MIN_PRI,
                               Thread::REALTIME,
                               1,
                               thread));

    // Wait a relatively long time to avoid racing the created thread.
    Clock::spinWait(0.1 * Clock::NS_IN_S);

    // Thread successfully wrote element `foo` since it's unlocked. `bar` is
    // unchanged since the unit test thread holds the lock.
    CHECK_EQUAL(100, elemFoo.read());
    CHECK_EQUAL(0, bar); // Direct read since already holding lock

    // Release lock and wait on other thread. It acquires the lock on `bar` and
    // writes it.
    CHECK_SUCCESS(lock.release());
    CHECK_SUCCESS(thread.await(nullptr));
    CHECK_EQUAL(100, elemBar.read());

    // Other thread released the lock on `bar` so it can be written again.
    elemBar.write(101);
    CHECK_EQUAL(101, bar);
}

///
/// @test Writing a region acquires and releases its lock.
///
TEST(StateVectorSync, RegionWriteSync)
{
    // Pin the current thread to core 0. The thread we create to contend the
    // region will go on core 1.
    CHECK_SUCCESS(Thread::set((Thread::REALTIME_MIN_PRI + 1),
                              Thread::REALTIME,
                              0));

    // Create and acquire spinlock.
    Spinlock lock;
    CHECK_SUCCESS(Spinlock::init(lock));
    CHECK_SUCCESS(lock.acquire());

    // Create region without lock.
    I32 foo = 0;
    Region regionFoo(&foo, sizeof(foo));

    // Create region with lock.
    I32 bar = 0;
    Region regionBar(&bar, sizeof(bar), &lock);

    // Create thread to write both regions.
    Thread thread;
    ThreadArgs args = {nullptr, nullptr, &regionFoo, &regionBar};
    CHECK_SUCCESS(Thread::init(writeRegionThread,
                               &args,
                               Thread::REALTIME_MIN_PRI,
                               Thread::REALTIME,
                               1,
                               thread));

    // Wait a relatively long time to avoid racing the created thread.
    Clock::spinWait(0.1 * Clock::NS_IN_S);

    // Thread successfully wrote region `foo` since it's unlocked. `bar` is
    // unchanged since the unit test thread holds the lock.
    CHECK_EQUAL(100, foo);
    CHECK_EQUAL(0, bar);

    // Release lock and wait on other thread. It acquires the lock on `bar` and
    // and writes it.
    CHECK_SUCCESS(lock.release());
    CHECK_SUCCESS(thread.await(nullptr));
    CHECK_EQUAL(100, bar);

    // Other thread released the lock on `bar` so it can be written again.
    I32 val = 101;
    CHECK_SUCCESS(regionBar.write(&val, sizeof(val)));
    CHECK_EQUAL(101, bar);
}

///
/// @test Reading an element acquires and releases its lock.
///
TEST(StateVectorSync, ElementReadSync)
{
    // Create and acquire spinlock.
    Spinlock lock;
    CHECK_SUCCESS(Spinlock::init(lock));
    CHECK_SUCCESS(lock.acquire());

    // Create element with lock.
    I32 foo = 0;
    Element<I32> elem(foo, &lock);

    // Create thread to read element.
    Thread thread;
    CHECK_SUCCESS(Thread::init(readElemThread,
                               &elem,
                               Thread::REALTIME_MIN_PRI,
                               Thread::REALTIME,
                               1,
                               thread));

    // Wait a relatively long time to avoid racing thread creation.
    Clock::spinWait(0.1 * Clock::NS_IN_S);

    // Update element value. Write to the backing directly since the element
    // lock is already held.
    foo = 100;

    // Release lock and wait on other thread. It returns the updated element
    // value since it was waiting to acquire the lock.
    CHECK_SUCCESS(lock.release());
    Result threadRes = 0;
    CHECK_SUCCESS(thread.await(&threadRes));
    CHECK_EQUAL(100, threadRes);

    // Other thread released the element lock so it can be read again.
    CHECK_EQUAL(100, elem.read());
}

///
/// @test Reading a region acquires and releases its lock.
///
TEST(StateVectorSync, RegionReadSync)
{
    // Create and acquire spinlock.
    Spinlock lock;
    CHECK_SUCCESS(Spinlock::init(lock));
    CHECK_SUCCESS(lock.acquire());

    // Create element with lock.
    I32 foo = 0;
    Region region(&foo, sizeof(foo), &lock);

    // Create thread to read region.
    Thread thread;
    CHECK_SUCCESS(Thread::init(readRegionThread,
                               &region,
                               Thread::REALTIME_MIN_PRI,
                               Thread::REALTIME,
                               1,
                               thread));

    // Wait a relatively long time to avoid racing thread creation.
    Clock::spinWait(0.1 * Clock::NS_IN_S);

    // Update region value. Write to the backing directly since the region
    // lock is already held.
    foo = 100;

    // Release lock and wait on other thread. It returns the updated region
    // value since it was waiting to acquire the lock.
    CHECK_SUCCESS(lock.release());
    Result threadRes = 0;
    CHECK_SUCCESS(thread.await(&threadRes));
    CHECK_EQUAL(100, threadRes);

    // Other thread released the region lock so it can be read again.
    I32 val = 0;
    CHECK_SUCCESS(region.read(&val, sizeof(val)));
    CHECK_EQUAL(100, val);
}

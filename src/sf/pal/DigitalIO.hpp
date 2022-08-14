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
/// @file  sf/pal/DigitalIO.hpp
/// @brief Platform-agnostic interface for accessing digital I/O pin hardware.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_DIGITAL_IO_HPP
#define SF_DIGITAL_IO_HPP

#include "sf/core/BasicTypes.hpp"
#include "sf/core/Result.hpp"

#ifdef SF_PLATFORM_SBRIO9637
#    include "sf/psl/sbrio9637/NiFpgaSession.hpp"
#endif

///
/// @brief Platform-agnostic interface for accessing digital I/O pin hardware.
///
/// DigitalIO defines the interface which the framework API layer uses to access
/// digital I/O pins on the target platform. The interface is designed to be
/// general and cross-platform, and not all methods or arguments may be used on
/// certain platforms. PSL implementations of DigitalIO have some freedom to
/// interpret the interface but should adhere to the language of the interface
/// docstrings as closely as possible.
///
/// "Digital" in the context of this interface refers to a discrete low/false or
/// high/true signal. The voltage of a digital high is implementation-defined. A
/// digital low is always 0 V. DigitalIO is intended for basic digital I/O
/// (e.g., event trigger lines) and not any kind of serial protocols like UART.
/// The DigitalIO interface is designed for use with tristate GPIO, where each
/// digital pin can be configured as input or output at runtime.
///
/// DigitalIO uses the same factory method and RAII patterns as most objects in
/// the framework. The user default-constructs a DigitalIO and then passes it to
/// a factory method that initializes it. The "resources" represented by a
/// DigitalIO are any digital highs it writes. These highs are tied to the
/// lifetime of the DigitalIO and are lowered when it destructs.
///
class DigitalIO final
{
public:

    ///
    /// @brief Possible modes for a digital pin.
    ///
    enum Mode : U8
    {
        IN = 0, ///< Input pin.
        OUT = 1 ///< Output pin.
    };

    ///
    /// @brief Initializes a DigitalIO.
    ///
    /// @pre  kDio is uninitialized.
    /// @post On success, kDio is initialized and invoking methods on it may
    ///       succeed.
    /// @post On error, preconditions still hold.
    /// @post The DIO hardware state is indeterminate.
    ///
    /// @param[in] kDio  DigitalIO to initialize.
    ///
    /// @retval SUCCESS       Successfully initialized.
    /// @retval E_DIO_REINIT  kDio is already initialized.
    /// @retval [other]       Initialization failed.
    ///
    static Result init(DigitalIO& kDio);

    ///
    /// @brief Default constructor.
    ///
    /// @post The constructed DigitalIO is uninitialized and invoking any of its
    /// methods returns an error.
    ///
    DigitalIO();

    ///
    /// @brief Destructor.
    ///
    /// @post If the DigitalIO was initialized, digital outputs it wrote during
    /// its initialized lifetime are lowered.
    ///
    /// @see DigitalIO::release()
    ///
    ~DigitalIO();

    ///
    /// @brief Sets the mode of a digital pin.
    ///
    /// @param[in] kPin   Pin number.
    /// @param[in] kMode  Requested pin mode.
    ///
    /// @retval SUCCESS       Successfully set pin mode.
    /// @retval E_DIO_UNINIT  DigitalIO is uninitialized.
    /// @retval E_DIO_PIN     kPin is invalid.
    /// @retval E_DIO_MODE    kMode is invalid.
    ///
    Result setMode(const U32 kPin, const DigitalIO::Mode kMode);

    ///
    /// @brief Reads a digital pin.
    ///
    /// @note This method is not necessarily restricted to pins configured as
    /// digital inputs. On platforms with tristate GPIO, reading a digital
    /// output pin may read the last value written to the pin.
    ///
    /// @post On success, kVal contains the read value.
    /// @post On error, kVal is unchanged.
    ///
    /// @param[in]  kPin  Pin number.
    /// @param[out] kVal  Reference to assign read value. True represents
    ///                   digital high, and false represents digital low.
    ///
    /// @retval SUCCESS       Successfully read pin.
    /// @retval E_DIO_UNINIT  DigitalIO is uninitialized.
    /// @retval E_DIO_PIN     kPin is invalid.
    ///
    Result read(const U32 kPin, bool& kVal);

    ///
    /// @brief Writes a digital output pin.
    ///
    /// @post On success, pin is outputting the specified value.
    /// @post On error, output of the pin is unchanged.
    ///
    /// @param[in] kPin  Pin number.
    /// @param[in] kVal  Requested pin output value. True represents digital
    ///                  high, and false represents digital low.
    ///
    /// @retval SUCCESS       Successfully wrote pin.
    /// @retval E_DIO_UNINIT  DigitalIO is uninitialized.
    /// @retval E_DIO_PIN     kPin is invalid.
    ///
    Result write(const U32 kPin, const bool kVal);

    ///
    /// @brief Releases the DigitalIO's resources and uninitializes it. The
    /// DigitalIO may be initialized again afterwards.
    ///
    /// @post Digital outputs written by the DigitalIO during its initialized
    /// lifetime are set back to zero.
    ///
    /// @retval SUCCESS  Successfully released.
    ///
    Result release();

private:

    ///
    /// @brief Whether DigitalIO is initialized.
    ///
    bool mInit;

#ifdef SF_PLATFORM_ARDUINO

    ///
    /// @brief Bit vector of pin output values. The rightmost bit stores the
    /// last value written to pin 0, the 2nd rightmost stores pin 1, and so on.
    /// This is used to lower all pins raised by the DigitalIO when it is
    /// released.
    ///
    U64 mOutBitVec;

#elif defined(SF_PLATFORM_SBRIO9637)

    ///
    /// @brief FPGA session handle.
    ///
    NiFpga_Session mSession;

#endif
};

#endif

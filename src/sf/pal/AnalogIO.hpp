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
/// @file  sf/pal/AnalogIO.hpp
/// @brief Platform-agnostic interface for accessing analog I/O pin hardware.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_ANALOG_IO_HPP
#define SF_ANALOG_IO_HPP

#include "sf/core/BasicTypes.hpp"
#include "sf/core/Result.hpp"

#ifdef SF_PLATFORM_SBRIO9637
#    include "sf/psl/sbrio9637/NiFpgaSession.hpp"
#endif

///
/// @brief Platform-agnostic interface for accessing analog I/O pin hardware.
///
/// AnalogIO defines the interface which the framework API layer uses to access
/// analog I/O pins on the target platform. The interface is designed to be
/// general and cross-platform, and not all methods or arguments may be used on
/// certain platforms. PSL implementations of AnalogIO have some freedom to
/// interpret the interface but should adhere to the language of the interface
/// docstrings as closely as possible.
///
/// "Analog" in the context of this interface is implementation-defined as well.
/// It will usually refer to a signal voltage but could also be a signal
/// current, PWM signal, or something else.
///
/// AnalogIO uses the same factory method and RAII patterns as most objects in
/// the framework. The user default-constructs an AnalogIO and then passes it
/// to a factory method that initializes it. The "resources" represented by an
/// AnalogIO are any analog output signals it writes. These signals are tied to
/// the lifetime of the AnalogIO and are set back to zero when it destructs.
///
class AnalogIO final
{
public:

    ///
    /// @brief Initializes an AnalogIO.
    ///
    /// @pre  kAio is uninitialized.
    /// @post On success, kAio is initialized and invoking methods on it may
    ///       succeed.
    /// @post On error, preconditions still hold.
    /// @post The AIO hardware state is indeterminate.
    ///
    /// @param[in] kAio  AnalogIO to initialize.
    ///
    /// @retval SUCCESS       Successfully initialized.
    /// @retval E_AIO_REINIT  kAio is already initialized.
    /// @retval [other]       Initialization failed.
    ///
    static Result init(AnalogIO& kAio);

    ///
    /// @brief Default constructor.
    ///
    /// @post The constructed AnalogIO is uninitialized and invoking any of its
    /// methods returns an error.
    ///
    AnalogIO();

    ///
    /// @brief Destructor.
    ///
    /// @post If the AnalogIO was initialized, analog outputs it wrote during
    /// its initialized lifetime are set to 0.
    ///
    /// @see AnalogIO::release()
    ///
    ~AnalogIO();

    ///
    /// @brief Sets the mode of an analog pin.
    ///
    /// The meaning of "mode" is implementation-defined. The most common
    /// interpretation is probably analog input mode, e.g., RSE vs.
    /// differential.
    ///
    /// @note sbRIO-9637: kPin is an input pin. Mode 1 is RSE, and mode 0 is
    /// differential. A differential pin's reference is the pin 8 above it. Pins
    /// >=8 should not be read in differential mode, as the read value will be
    /// negated.
    ///
    /// @param[in] kPin   Pin number.
    /// @param[in] kMode  Pin mode. The meaning of this value is
    ///                   implementation-defined.
    ///
    /// @retval SUCCESS       Successfully set pin mode.
    /// @retval E_AIO_UNINIT  AnalogIO is uninitialized.
    /// @retval E_AIO_PIN     kPin is invalid.
    /// @retval E_AIO_MODE    kMode is invalid.
    ///
    Result setMode(const U32 kPin, const I8 kMode);

    ///
    /// @brief Sets the input/output range of an analog pin.
    ///
    /// @note sbRIO-9637: Valid ranges are 1, 2, 5, and 10 for +/- that many V.
    ///
    /// @param[in] kPin    Pin number.
    /// @param[in] kRange  Pin range. The meaning of this value is
    ///                    implementation-defined.
    ///
    /// @retval SUCCESS       Successfully set pin range.
    /// @retval E_AIO_UNINIT  AnalogIO is uninitialized.
    /// @retval E_AIO_PIN     kPin is invalid.
    /// @retval E_AIO_RANGE   kRange is invalid.
    ///
    Result setRange(const U32 kPin, const I8 kRange);

    ///
    /// @brief Reads an analog input pin.
    ///
    /// @post On success, kVal contains the read value.
    /// @post On error, kVal is unchanged.
    ///
    /// @note sbRIO-9637: The unit of kVal is V.
    ///
    /// @param[in]  kPin  Pin number.
    /// @param[out] kVal  Reference to assign read value. The meaning of this
    ///                   value is implementation-defined.
    ///
    /// @retval SUCCESS       Successfully read pin.
    /// @retval E_AIO_UNINIT  AnalogIO is uninitialized.
    /// @retval E_AIO_PIN     kPin is invalid.
    ///
    Result read(const U32 kPin, F32& kVal);

    ///
    /// @brief Writes an analog output pin.
    ///
    /// @post On success, pin is outputting the specified value.
    /// @post On error, output of the pin is unchanged.
    ///
    /// @note sbRIO-9637: The unit of kVal is V.
    ///
    /// @param[in] kPin  Pin number.
    /// @param[in] kVal  Pin output value. The meaning of this value is
    ///                  implementation-defined.
    ///
    /// @retval SUCCESS       Successfully wrote pin.
    /// @retval E_AIO_UNINIT  AnalogIO is uninitialized.
    /// @retval E_AIO_PIN     kPin is invalid.
    /// @retval E_AIO_OUT     kVal is invalid.
    ///
    Result write(const U32 kPin, const F32 kVal);

    ///
    /// @brief Releases the AnalogIO's resources and uninitializes it. The
    /// AnalogIO may be initialized again afterwards.
    ///
    /// @post Analog outputs written by the AnalogIO during its initialized
    /// lifetime are set back to zero.
    ///
    /// @retval SUCCESS  Successfully released.
    ///
    Result release();

private:

    ///
    /// @brief Whether AnalogIO is initialized.
    ///
    bool mInit;

#ifdef SF_PLATFORM_SBRIO9637

    ///
    /// @brief FPGA session handle.
    ///
    NiFpga_Session mSession;

#endif
};

#endif

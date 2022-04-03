////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building avionics
/// software applications. Built in Austin, Texas at the University of Texas at
/// Austin. Surefire is open-source under the Apache License 2.0 - a copy of the
/// license may be obtained at https://www.apache.org/licenses/LICENSE-2.0.
/// Surefire is maintained at https://www.github.com/stefandebruyn/surefire.
///
///                             ---------------
/// @file  sf/pal/DigitalIO.hpp
/// @brief Platform-agnostic interface for accessing digital I/O pin hardware.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_DIGITAL_IO_HPP
#define SF_DIGITAL_IO_HPP

#include "sf/core/BasicTypes.hpp"
#include "sf/core/Result.hpp"

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
/// a factory method that initializes it. The "resources" acquired by a
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
        ///
        /// @brief Input pin.
        ///
        IN = 0,

        ///
        /// @brief Output pin.
        ///
        OUT = 1
    };

    ///
    /// @brief Initializes a DigitalIO.
    ///
    /// @pre  kDio is uninitialized.
    /// @post On success, kDio is initialized and invoking methods on it may
    ///       succeed.
    /// @post On error, preconditions still hold.
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
    /// @post Digital outputs written by the DigitalIO during its initialized
    /// lifetime are set back to zero.
    ///
    ~DigitalIO();

    ///
    /// @brief Sets the mode of a digital pin.
    ///
    /// @param[in] kPin   Pin number.
    /// @param[in] kMode  Requested pin mode.
    ///
    /// @retval SUCCESS     Successfully set pin mode.
    /// @retval E_DIO_PIN   Invalid pin.
    /// @retval E_DIO_MODE  Invalid mode.
    ///
    Result setMode(const U32 kPin, const Mode kMode);

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
    /// @param[out] kVal  Reference to populate with read value. True is equal
    ///                   to digital high, and false, digital low.
    ///
    /// @retval SUCCESS    Successfully read pin.
    /// @retval E_DIO_PIN  Invalid pin.
    ///
    Result read(const U32 kPin, bool& kVal);

    ///
    /// @brief Writes a digital output pin.
    ///
    /// @post On success, pin is outputting the specified value.
    /// @post On error, output of the pin is unchanged.
    ///
    /// @param[in] kPin  Pin number.
    /// @param[in] kVal  Requested pin output value. True is equal to digital
    ///                  high, and false, digital low.
    ///
    /// @retval SUCCESS    Successfully wrote pin.
    /// @retval E_DIO_PIN  Invalid pin.
    ///
    Result write(const U32 kPin, const bool kVal);

    ///
    /// @brief Releases the DigitalIO's resources and uninitializes it, allowing
    /// it to be initialized again later.
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
    /// Bit vector of pin output values. The rightmost bit stores the last value
    /// written to pin 0, the 2nd rightmost stores pin 1, and so on. This is
    /// used to lower all pins raised by the DigitalIO when it is released.
    U64 mOutBitVec;
#endif
};

#endif

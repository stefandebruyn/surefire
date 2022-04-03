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
/// @file  sf/pal/AnalogIO.hpp
/// @brief Platform-agnostic interface for accessing analog I/O pin hardware.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_ANALOG_IO_HPP
#define SF_ANALOG_IO_HPP

#include "sf/core/BasicTypes.hpp"
#include "sf/core/Result.hpp"

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
/// It will usually refer to a signal voltage but could also be a current, PWM
/// signal, or something else.
///
/// AnalogIO uses the same factory method and RAII patterns as most objects in
/// the framework. The user default-constructs an AnalogIO and then passes it
/// to a factory method that initializes it. The "resources" acquired by an
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
    /// @post Analog outputs written by the AnalogIO during its initialized
    /// lifetime are set back to zero.
    ///
    ~AnalogIO();

    ///
    /// @brief Sets the input/output range of an analog pin.
    ///
    /// @remark Embedded platforms that allow configuring analog I/O ranges in
    /// software are somewhat uncommon, so this method will be rarely
    /// implemented.
    ///
    /// @param[in] kPin    Pin number.
    /// @param[in] kRange  Requested pin range. The meaning of this value is
    ///                    implementation-defined.
    ///
    /// @retval SUCCESS      Successfully set pin range.
    /// @retval E_AIO_PIN    Invalid pin.
    /// @retval E_AIO_RANGE  Invalid range.
    ///
    Result setRange(const U32 kPin, const I8 kRange);

    ///
    /// @brief Reads an analog input pin.
    ///
    /// @post On success, kVal contains the read value.
    /// @post On error, kVal is unchanged.
    ///
    /// @param[in]  kPin  Pin number.
    /// @param[out] kVal  Reference to populate with read value. The meaning of
    ///                   this value is implementation-defined.
    ///
    /// @retval SUCCESS    Successfully read pin.
    /// @retval E_AIO_PIN  Invalid pin.
    ///
    Result read(const U32 kPin, F32& kVal);

    ///
    /// @brief Writes an analog output pin.
    ///
    /// @post On success, pin is outputting the specified value.
    /// @post On error, output of the pin is unchanged.
    ///
    /// @param[in] kPin  Pin number.
    /// @param[in] kVal  Requested pin output value. The meaning of this value
    ///                  is implementation-defined.
    ///
    /// @retval SUCCESS    Successfully wrote pin.
    /// @retval E_AIO_PIN  Invalid pin.
    /// @retval E_AIO_OUT  Invalid output value.
    ///
    Result write(const U32 kPin, const F32 kVal);

    ///
    /// @brief Releases the AnalogIO's resources and uninitializes it, allowing
    /// it to be initialized again later.
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
};

#endif

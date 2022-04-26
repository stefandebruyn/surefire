# Toolchain file for cross-compiling to NILRT. Assumes a Linux build host with
# the NILRT SDK installed.

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_C_COMPILER /usr/local/oecore-x86_64/sysroots/x86_64-nilrtsdk-linux/usr/bin/arm-nilrt-linux-gnueabi/arm-nilrt-linux-gnueabi-gcc)
set(CMAKE_CXX_COMPILER /usr/local/oecore-x86_64/sysroots/x86_64-nilrtsdk-linux/usr/bin/arm-nilrt-linux-gnueabi/arm-nilrt-linux-gnueabi-g++)
set(SF_PLATFORM_NILRT true)

# Note the `SF_PLATFORM_NILRT` symbol: this will trigger small code changes in
# the Linux PSL.
add_compile_options(
    -DSF_PLATFORM_NILRT
    -O0
    -g3
    -Wall
    -mfpu=vfpv3
    -mfloat-abi=softfp
    --sysroot=/usr/local/oecore-x86_64/sysroots/cortexa9-vfpv3-nilrt-linux-gnueabi
)
add_link_options(--sysroot=/usr/local/oecore-x86_64/sysroots/cortexa9-vfpv3-nilrt-linux-gnueabi)
link_libraries(dl)

set(CMAKE_SYSTEM_NAME Linux)

# NILRT SDK compiler locations on a Linux build host.
set(CMAKE_C_COMPILER /usr/local/oecore-x86_64/sysroots/x86_64-nilrtsdk-linux/usr/bin/arm-nilrt-linux-gnueabi/arm-nilrt-linux-gnueabi-gcc)
set(CMAKE_CXX_COMPILER /usr/local/oecore-x86_64/sysroots/x86_64-nilrtsdk-linux/usr/bin/arm-nilrt-linux-gnueabi/arm-nilrt-linux-gnueabi-g++)

add_compile_options(
    -O0
    -g3
    -Wall
    -mfpu=vfpv3
    -mfloat-abi=softfp
    --sysroot=/usr/local/oecore-x86_64/sysroots/cortexa9-vfpv3-nilrt-linux-gnueabi
)
add_link_options(--sysroot=/usr/local/oecore-x86_64/sysroots/cortexa9-vfpv3-nilrt-linux-gnueabi)

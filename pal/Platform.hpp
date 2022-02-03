#ifndef SFA_PLATFORM_HPP
#define SFA_PLATFORM_HPP

#if __linux__
#    define SFA_PLATFORM_LINUX
#else
#    error unable to determine platform
#endif

#endif

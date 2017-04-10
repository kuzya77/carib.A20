AC_PREREQ([2.69])

AC_INIT([carib], [1.5.0])

AM_INIT_AUTOMAKE([foreign])

AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h malloc.h stddef.h stdlib.h string.h sys/ioctl.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_MMAP
AC_CHECK_FUNCS([ftruncate memset mkdir munmap select sqrt strchr strerror strpbrk strrchr strspn strtol])

AC_CONFIG_FILES([Makefile
                 engine/Makefile
                 game/Makefile
                 utils/Makefile])

AC_OUTPUT


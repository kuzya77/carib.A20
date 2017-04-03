AC_INIT([carib], [1.5.0])

AM_INIT_AUTOMAKE

AC_PROG_CXX
AC_PROG_RANLIB

AC_CONFIG_HEADERS([config.h])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE

# Checks for library functions.
AC_CHECK_FUNCS([strchr strrchr])

AC_CONFIG_FILES([Makefile
                 engine/Makefile
                 game/Makefile])

AC_OUTPUT


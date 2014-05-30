

AC_INIT([bluetooth], [1.0], [bug-report@address])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_LANG_C
AC_PROG_CC
AM_PROG_CC_C_O

# Add an option to produce statically linked binaries.

AC_CONFIG_MACRO_DIR([m4])
PKG_CHECK_MODULES([BlueZ], [bluez])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile APP_BLUETOOTH/Makefile])
AC_OUTPUT



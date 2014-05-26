

AC_INIT([bluetooth], [1.0], [bug-report@address])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_LANG_C
AC_PROG_CC

#AC_CHECK_LIB([bluetooth],[hci_get_route],[LIBBLUEZ=-lbluetooth],[AC_MSG_ERROR([No se ha podido encontrar la libreria  de bluetooth])])
AC_CHECK_LIB([bluetooth],[hci_get_route],[LIBBLUEZ=-lbluetooth])
AC_SUBST([LIBBLUEZ])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile APP_BLUETOOTH/Makefile])
AC_OUTPUT



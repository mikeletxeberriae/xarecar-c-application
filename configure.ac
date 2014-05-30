

AC_INIT([bluetooth], [1.0], [bug-report@address])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_LANG_C
AC_PROG_CC
AM_PROG_CC_C_O

# Add an option to produce statically linked binaries.
AC_ARG_ENABLE([static],
               AC_HELP_STRING([--enable-static],
                              [build static library @<:@default=no@:>@]),
              [static=$enableval],
              [static=no])


#AC_PROG_LIBTOOL 
AC_CONFIG_MACRO_DIR([m4])
PKG_CHECK_MODULES([BlueZ], [bluez])

# Check whether user supplied the option to statically link binaries.
if test "$static" = yes; then      
       SAVED_LDFLAGS="-static"
else
       SAVED_LDFLAGS=" "
fi


AC_SUBST(SAVED_LDFLAGS)

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile APP_BLUETOOTH/Makefile])
AC_OUTPUT



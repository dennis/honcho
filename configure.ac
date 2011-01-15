AC_PREREQ(2.59)

AC_INIT(honcho, 0.0.1, dennis@moellegaard.dk)
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADER([src/config.h])

AM_INIT_AUTOMAKE($PACKAGE_NAME, $PACKAGE_VERSION)

AC_PROG_LIBTOOL

AC_DEFINE(QUEUE_DIR, "./queue/", [Default queue directory])
AC_DEFINE(FILE_MODE, 0666, [Base file permission])
AC_DEFINE(DIR_MODE, 0777, [Base directory permission])
AC_DEFINE(SHELL_BIN, "/bin/sh", [Shell binary])

AC_ARG_ENABLE([debug],
              AC_HELP_STRING([--enable-debug],
              [compile with debug symbols @<:@default=no@:>@]),
              [want_debug="$enableval"], [want_debug=no])

if test "x$want_debug" = "xyes" -a $ac_cv_c_compiler_gnu != no; then
	AC_MSG_NOTICE([Debugging enabled])
	CFLAGS="-O0 -ggdb"
	AC_DEFINE([DEBUG], 1, [Define for debugging])
fi

AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT

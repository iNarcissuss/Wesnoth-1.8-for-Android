# absolute-header.m4 serial 11
dnl Copyright (C) 2006-2010 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Derek Price.

# gl_ABSOLUTE_HEADER(HEADER1 HEADER2 ...)
# ---------------------------------------
# Find the absolute name of a header file, assuming the header exists.
# If the header were sys/inttypes.h, this macro would define
# ABSOLUTE_SYS_INTTYPES_H to the `""' quoted absolute name of sys/inttypes.h
# in config.h
# (e.g. `#define ABSOLUTE_SYS_INTTYPES_H "///usr/include/sys/inttypes.h"').
# The three "///" are to pacify Sun C 5.8, which otherwise would say
# "warning: #include of /usr/include/... may be non-portable".
# Use `""', not `<>', so that the /// cannot be confused with a C99 comment.
# Note: This macro assumes that the header file is not empty after
# preprocessing, i.e. it does not only define preprocessor macros but also
# provides some type/enum definitions or function/variable declarations.
AC_DEFUN([gl_ABSOLUTE_HEADER],
[AC_REQUIRE([AC_CANONICAL_HOST])
AC_LANG_PREPROC_REQUIRE()dnl
m4_foreach_w([gl_HEADER_NAME], [$1],
  [AS_VAR_PUSHDEF([gl_absolute_header],
                  [gl_cv_absolute_]m4_defn([gl_HEADER_NAME]))dnl
  AC_CACHE_CHECK([absolute name of <]m4_defn([gl_HEADER_NAME])[>],
    m4_defn([gl_absolute_header]),
    [AS_VAR_PUSHDEF([ac_header_exists],
                    [ac_cv_header_]m4_defn([gl_HEADER_NAME]))dnl
    AC_CHECK_HEADERS_ONCE(m4_defn([gl_HEADER_NAME]))dnl
    if test AS_VAR_GET(ac_header_exists) = yes; then
      AC_LANG_CONFTEST([AC_LANG_SOURCE([[#include <]]m4_dquote(m4_defn([gl_HEADER_NAME]))[[>]])])
      dnl AIX "xlc -E" and "cc -E" omit #line directives for header files
      dnl that contain only a #include of other header files and no
      dnl non-comment tokens of their own. This leads to a failure to
      dnl detect the absolute name of <dirent.h>, <signal.h>, <poll.h>
      dnl and others. The workaround is to force preservation of comments
      dnl through option -C. This ensures all necessary #line directives
      dnl are present. GCC supports option -C as well.
      case "$host_os" in
        aix*) gl_absname_cpp="$ac_cpp -C" ;;
        *)    gl_absname_cpp="$ac_cpp" ;;
      esac
      dnl eval is necessary to expand gl_absname_cpp.
      dnl Ultrix and Pyramid sh refuse to redirect output of eval,
      dnl so use subshell.
      AS_VAR_SET(gl_absolute_header,
[`(eval "$gl_absname_cpp conftest.$ac_ext") 2>&AS_MESSAGE_LOG_FD |
sed -n '\#/]m4_defn([gl_HEADER_NAME])[#{
        s#.*"\(.*/]m4_defn([gl_HEADER_NAME])[\)".*#\1#
        s#^/[^/]#//&#
        p
        q
}'`])
    fi
    AS_VAR_POPDEF([ac_header_exists])dnl
    ])dnl
  AC_DEFINE_UNQUOTED(AS_TR_CPP([ABSOLUTE_]m4_defn([gl_HEADER_NAME])),
                     ["AS_VAR_GET(gl_absolute_header)"],
                     [Define this to an absolute name of <]m4_defn([gl_HEADER_NAME])[>.])
  AS_VAR_POPDEF([gl_absolute_header])dnl
])dnl
])# gl_ABSOLUTE_HEADER

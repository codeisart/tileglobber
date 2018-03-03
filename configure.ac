AC_INIT([globber], [0.1], [jimmy@doomkeeper.com])
AC_PREREQ([2.69])
AM_INIT_AUTOMAKE([foreign -Wall -Werror subdir-objects])

# store current user given compiler flags to avoid default setup via AC_PROG_CXX
OLD_CXXFLAGS=$CXXFLAGS

# check for C++ preprocessor and compiler.
AC_PROG_CXX
AC_PROG_CXXCPP

# reset compiler flags to initial flags
CXXFLAGS=$OLD_CXXFLAGS

# switch to c++ compiler for the following checks
AC_LANG(C++)

# check for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([string])
AC_CHECK_HEADERS([iostream])
AC_CHECK_HEADERS([memory])
AC_CHECK_HEADERS([iostream])
AC_CHECK_HEADERS([exception])
AC_CHECK_HEADERS([vector])
AC_CHECK_HEADERS([iomanip])

AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_PROG_CC

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Save cache state. So we dont have to do the above checks again
AC_CACHE_SAVE

# Link statically stdlib.
#AM_LDFLAGS="$AM_LDFLAGS"  # -static-libgcc"
AM_LDFLAGS="$AM_LDFLAGS" # -stdlib=libc++"
##########################################################################
# debug compilation support
##########################################################################

AC_MSG_CHECKING([whether to build with debug information])
AC_ARG_ENABLE([debug],
    [AS_HELP_STRING([--enable-debug],
        [enable debug data generation (def=no)])],
    [debugit="$enableval"],
    [debugit=no])
AC_MSG_RESULT([$debugit])

if test x"$debugit" = x"yes"; then
    AC_DEFINE([DEBUG],[],[Debug Mode])
    AM_CXXFLAGS="$AM_CXXFLAGS -ggdb -Wall -Werror -Wno-uninitialized -O0"
else
    AC_DEFINE([NDEBUG],[],[No-debug Mode])
    AM_CXXFLAGS="$AM_CXXFLAGS -O3"
fi

##########################################################################

# distribute additional compiler and linker flags among Makefiles
# --> set and change these variables instead of CXXFLAGS or LDFLAGS (for user only)
AC_SUBST([AM_CXXFLAGS])
AC_SUBST([AM_LDFLAGS])

# look in the m4 folder for extra macros.
# m4_include([m4/m4_ax_python_devel.m4])
AC_CONFIG_MACRO_DIR([m4])

# check for c++11 and unordered_map
AX_CXX_COMPILE_STDCXX_11
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
	Makefile 
	src/Makefile 
])
AC_OUTPUT

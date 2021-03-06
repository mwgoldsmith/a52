dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.54)
AC_INIT
AC_CONFIG_SRCDIR([src/a52dec.c])
AC_CONFIG_AUX_DIR(.auto)
AC_CONFIG_FILES([Makefile liba52/Makefile include/Makefile test/Makefile
    doc/Makefile src/Makefile libao/Makefile vc++/Makefile liba52/liba52.pc])
AM_INIT_AUTOMAKE([a52dec],[0.7.5-cvs])
AC_CONFIG_HEADERS([include/config.h])
AM_MAINTAINER_MODE
AC_CANONICAL_HOST

dnl Checks for compiler
AC_PROG_CC
AC_PROG_GCC_TRADITIONAL

dnl Checks for headers. We do this before the CC-specific section because
dnl autoconf generates tests for generic headers before the first header test.
AC_CHECK_HEADERS([sys/time.h time.h sys/timeb.h io.h])

dnl CC-specific flags
AC_SUBST([OPT_CFLAGS])
AC_SUBST([ARCH_OPT_CFLAGS])
if test x"$GCC" = x"yes"; then

    dnl GCC-specific flags

    dnl -Wall
    dnl -Werror moved to the end to not disturb the configure script
    TRY_CFLAGS="$OPT_CFLAGS -Wall -Wpointer-arith -Wcast-align -Wcast-qual -Wstrict-prototypes -Wshadow -Waggregate-return -Wmissing-prototypes -Wnested-externs -Wsign-compare"
    AC_TRY_CFLAGS([$TRY_CFLAGS $CFLAGS],[OPT_CFLAGS="$TRY_CFLAGS"])

    dnl -O3
    changequote(<<,>>)
    TRY_CFLAGS=`echo "$OPT_CFLAGS $CFLAGS"|sed "s/-O[0-9]*//g"`
    changequote([,])
    TRY_CFLAGS="$TRY_CFLAGS -O3"
    AC_TRY_CFLAGS([$TRY_CFLAGS],[OPT_CFLAGS="$TRY_CFLAGS"; CFLAGS=""])

    AC_ARG_ENABLE([debug],
	[  --enable-debug          debug mode configuration])
    if test x"$enable_debug" = x"yes"; then
	AC_DEFINE([DEBUG],,[debug mode configuration])
    else
	dnl -fomit-frame-pointer
	TRY_CFLAGS="$OPT_CFLAGS -fomit-frame-pointer"
	AC_TRY_CFLAGS([$TRY_CFLAGS $CFLAGS],[OPT_CFLAGS="$TRY_CFLAGS"])
    fi

    dnl -fno-common
    TRY_CFLAGS="$OPT_CFLAGS -fno-common"
    AC_TRY_CFLAGS([$TRY_CFLAGS $CFLAGS],[OPT_CFLAGS="$TRY_CFLAGS"])

    dnl arch-specific flags
    case "$host" in
    i?86-* | k?-* | x86_64-* | amd64-*)
	AC_DEFINE([ARCH_X86],,[x86 architecture])
	case "$host" in
	i386-*) TRY_CFLAGS="$OPT_CFLAGS -mtune=i386";;
	i486-*) TRY_CFLAGS="$OPT_CFLAGS -mtune=i486";;
	i586-*) TRY_CFLAGS="$OPT_CFLAGS -mtune=pentium";;
	i686-*) TRY_CFLAGS="$OPT_CFLAGS -mtune=pentiumpro";;
	k6-*)   TRY_CFLAGS="$OPT_CFLAGS -mtune=k6";;
	esac
	AC_TRY_CFLAGS([$TRY_CFLAGS $CFLAGS],[OPT_CFLAGS="$TRY_CFLAGS"]);;
    ppc-* | powerpc-*)
	have_altivec=no
	for TRY_CFLAGS in "-mpim-altivec -force_cpusubtype_ALL" -faltivec -maltivec -fvec; do
	    AC_TRY_CFLAGS([$OPT_CFLAGS $TRY_CFLAGS $CFLAGS],
		[save_CFLAGS="$CFLAGS"
		 CFLAGS="$OPT_CFLAGS $TRY_CFLAGS $CFLAGS"
		 AC_MSG_CHECKING([if <altivec.h> is needed])
		 AC_TRY_COMPILE([],
		    [typedef vector int t;
		     vec_ld(0, (unsigned char *)0);],
		    [have_altivec=yes; AC_MSG_RESULT(no)],
		    [AC_TRY_COMPILE([#include <altivec.h>],
			[typedef vector int t; vec_ld(0, (unsigned char *)0);],
			[AC_DEFINE([HAVE_ALTIVEC_H],,
			    [Define to 1 if you have the <altivec.h> header.])
			 have_altivec=yes; AC_MSG_RESULT(yes)],
			[AC_MSG_RESULT(unsupported)])])
		 CFLAGS="$save_CFLAGS"])
	    if test "$have_altivec" = "yes"; then
		ARCH_OPT_CFLAGS="$TRY_CFLAGS"
		AC_DEFINE([ARCH_PPC],,[ppc architecture])
		break
	    fi
	done;;
    sparc-* | sparc64-*)
	AC_DEFINE([ARCH_SPARC],,[sparc architecture])
	TRY_CFLAGS="$OPT_CFLAGS -mtune=ultrasparc"
	AC_TRY_CFLAGS([$TRY_CFLAGS $CFLAGS],[OPT_CFLAGS="$TRY_CFLAGS"]);;
    alpha*)
	AC_DEFINE([ARCH_ALPHA],,[alpha architecture]);;
    esac
elif test x"$CC" = x"tendracc"; then
    dnl TenDRA portability checking compiler
    TENDRA=yes
    OPT_CFLAGS="-Xp -Yansi -f`pwd`/include/tendra.h -DELIDE_CODE"
    enable_mlib=no
    enable_oss=no
    enable_solaris_audio=no
elif test x"$CC" = x"icc" -a x"`$CC -V 2>&1 | grep Intel`" != x""; then
    dnl Intel C++ compiler
    OPT_CFLAGS="-g -O3 -unroll -ip"; CFLAGS=""
else
    dnl non-gcc flags - we probably need exact configuration triplets here.
    case "$host" in
    sparc-sun-solaris*)
	TRY_CFLAGS="$OPT_CFLAGS -xCC -fast -xO5"
	AC_TRY_CFLAGS([$TRY_CFLAGS $CFLAGS],[OPT_CFLAGS="$TRY_CFLAGS"]);;
    esac
fi

dnl Checks for libtool - this must be done after we set cflags
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL

dnl Checks for libraries.

dnl Checks for header files.
INCLUDES='-I$(top_srcdir)/include -I$(top_builddir)/include'
AC_SUBST([INCLUDES])
AC_HEADER_TIME
AC_CHECK_TYPES([struct timeval], [], [],
    [#include <sys/time.h>
    #include <time.h>])
AC_CHECK_GENERATE_INTTYPES([include])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_ALWAYS_INLINE
AC_C_RESTRICT
AC_C_BUILTIN_EXPECT
AC_C_BIGENDIAN
AC_TYPE_SIZE_T
AC_TYPE_SIGNAL
AC_SYS_LARGEFILE

dnl Checks for library functions.
AC_CHECK_FUNCS([memalign gettimeofday ftime])

AC_SUBST([LIBA52_CFLAGS])
AC_SUBST([LIBA52_LIBS])

dnl avoid -fPIC when possible
AC_LIBTOOL_NON_PIC([LIBA52_CFLAGS="$LIBA52_CFLAGS -prefer-non-pic"])

AC_ARG_ENABLE([fixed],
    [  --enable-fixed          use fixed-point (integer-based) arithmetic])
AC_ARG_ENABLE([double],
    [  --enable-double         use double-precision samples])
AC_ARG_ENABLE([djbfft],
    [  --enable-djbfft         make a version using djbfft])

if test x"$enable_fixed" = x"yes"; then
    AC_DEFINE([LIBA52_FIXED],,[use fixed-point arithmetic])
elif test x"$enable_double" = x"yes"; then
    AC_DEFINE([LIBA52_DOUBLE],,[a52 sample precision])
fi
if test x"$enable_djbfft" = x"yes"; then
    AC_DEFINE([LIBA52_DJBFFT],,[liba52 djbfft support])
    LIBA52_LIBS="$LIBA52_LIBS -ldjbfft"
fi

AC_SUBST([LIBAO_LIBS])

dnl check for oss
AC_ARG_ENABLE([oss],
    [  --disable-oss           make a version not using oss])
if test x"$enable_oss" != x"no"; then
    case "$host" in
    *-linux*|*-openbsd*|*-freebsd*|*-netbsd*|*-bsdi*)
        AC_DEFINE([LIBAO_OSS],,[libao OSS support])
        AC_CHECK_LIB([ossaudio],[_oss_ioctl],
            [LIBAO_LIBS="$LIBAO_LIBS -lossaudio"]);;
    esac
fi

dnl check for solaris
AC_ARG_ENABLE([solaris-audio],
    [  --disable-solaris-audio make a version not using solaris audio])
if test x"$enable_solaris_audio" != x"no"; then
    case "$host" in
    *-sun-solaris*)
        AC_DEFINE([LIBAO_SOLARIS],,[libao solaris support]);;
    esac
fi

dnl check for al
AC_ARG_ENABLE([al-audio],
    [  --disable-al-audio      make a version not using al audio])
if test x"$enable_al" != x"no"; then
    case "$host" in
    *-sgi-irix*)
        LIBAO_LIBS="$LIBAO_LIBS -laudio"
        AC_DEFINE([LIBAO_AL],,[libao al support]);;
    esac
fi

dnl check for win32
AC_ARG_ENABLE([win],
    [  --disable-win           make a version not using win audio])
if test x"$enable_win" != x"no"; then
    case "$host" in
    *-mingw32*|*-cygwin)
        LIBAO_LIBS="$LIBAO_LIBS -lwinmm"
        AC_DEFINE([LIBAO_WIN],,[libao win support]);;
    esac
fi

AC_ARG_ENABLE([warnings],
    [  --enable-warnings       treat warnings as errors])
if test x"$enable_warnings" = x"yes" -a x"$GCC" = x"yes"; then
    dnl compiler warnings
    TRY_CFLAGS="$OPT_CFLAGS -Werror"
    AC_TRY_CFLAGS([$TRY_CFLAGS $CFLAGS],[OPT_CFLAGS="$TRY_CFLAGS"])
elif test x"$TENDRA" = x"yes"; then
    dnl TenDRA portability checking compiler
    OPT_CFLAGS="$OPT_CFLAGS -DTenDRA_check"
fi

AC_SUBST([A52DEC_CFLAGS])

A52DEC_CFLAGS="$OPT_CFLAGS"

AC_ARG_ENABLE([gprof],[  --enable-gprof          make a version using gprof])
if test x"$enable_gprof" = x"yes"; then
    AC_DEFINE([A52DEC_GPROF],,[a52dec profiling])
    A52DEC_CFLAGS=`echo "$A52DEC_CFLAGS"|sed "s/-fomit-frame-pointer//g"`
    A52DEC_CFLAGS="$A52DEC_CFLAGS -p"
fi

AC_C_ATTRIBUTE_ALIGNED

AC_OUTPUT

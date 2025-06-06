dnl Copyright 1996-2003,2005,2006,2008,2009,2022 Alain Knaff.
dnl This file is part of mtools.
dnl
dnl Mtools is free software: you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation, either version 3 of the License, or
dnl (at your option) any later version.
dnl
dnl Mtools is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with Mtools.  If not, see <http://www.gnu.org/licenses/>.
dnl
dnl Process this file with autoconf to produce a configure script.
AC_INIT

AC_CONFIG_SRCDIR([buffer.c])

AC_USE_SYSTEM_EXTENSIONS dnl for strndup

dnl Checks for compiler
AC_PROG_CC

dnl Get canonical host
AC_CANONICAL_HOST

AC_PROG_INSTALL
AC_PROG_LN_S

AC_PATH_PROG(INSTALL_INFO, install-info, "")


AC_C_CONST
AC_C_INLINE

dnl Check for configuration options
dnl Enable OS/2 extended density format disks
AC_ARG_ENABLE(xdf,
[  --enable-xdf           support for OS/2 extended density format disks],
[if test x$enableval = xyes; then
  AC_DEFINE([USE_XDF],1,[Define this if you want to use Xdf])
  XDF_IO_SRC=xdf_io.c
  XDF_IO_OBJ=xdf_io.o
fi],[AC_DEFINE([USE_XDF],1,[Define this if you want to use Xdf])
XDF_IO_SRC=xdf_io.c
XDF_IO_OBJ=xdf_io.o
]
)
AC_SUBST(XDF_IO_SRC)
AC_SUBST(XDF_IO_OBJ)

dnl Check for configuration options
dnl Enable usage of vold on Solaris
AC_ARG_ENABLE(vold,
[  --enable-vold          compatibility with Solaris' vold],
[if test x$enableval = xyes; then
  AC_DEFINE([USING_VOLD],1,[Define this if you use mtools together with Solaris' vold])
fi])


dnl Check for configuration options
dnl Enable usage of vold on Solaris
AC_ARG_ENABLE(new-vold,
[  --enable-new-vold          compatibility with Solaris' vold, new version],
[newVold=x$enableval
if test x$enableval = xyes; then
  AC_DEFINE([USING_NEW_VOLD],1,[Define this if you use mtools together with the new Solaris' vold support])
fi])


dnl Check for configuration options
dnl Debugging
AC_ARG_ENABLE(debug,
[  --enable-debug         debugging messages],
[if test x$enableval = xyes; then
  AC_DEFINE([DEBUG],1,[Define for debugging messages])
fi])


dnl Check for configuration options
dnl Raw terminal code (enabled by default)
AC_ARG_ENABLE(raw_term,
[  --enable-raw-term      raw terminal (readkey behaviour, default)],
[if test x$enableval = xyes; then
  AC_DEFINE([USE_RAWTERM],1,[Define on non Unix OS'es which don't have the concept of tty's])
fi],
AC_DEFINE([USE_RAWTERM],1,[Define on non Unix OS'es which don't have the concept of tty's]))


dnl Checks for libraries.

dnl AC_FUNC_GETMNTENT
AC_CHECK_LIB([sun],[getpwnam])

case $host_os in
 solaris*)
    AC_CHECK_FUNCS(media_oldaliases)
    ;;
 freebsd*)
    AC_CHECK_LIB(cam, cam_open_device)
    ;;
esac

AC_CHECK_LIB(iconv, iconv)


dnl Checks for header files.
AC_HEADER_SYS_WAIT
AC_HEADER_STDBOOL
AC_HEADER_MAJOR
AC_CHECK_HEADERS(getopt.h stdarg.h \
libc.h fcntl.h limits.h sys/file.h sys/ioctl.h time.h sys/time.h \
sys/param.h memory.h malloc.h io.h signal.h sys/signal.h utime.h sgtty.h \
sys/floppy.h mntent.h sys/sysmacros.h assert.h \
iconv.h wctype.h wchar.h locale.h xlocale.h )
AC_CHECK_HEADERS(dirent.h direct.h, [break])
AC_CHECK_HEADERS(termio.h sys/termio.h, [break])
AC_CHECK_HEADERS(termios.h sys/termios.h, [break])

dnl Check for platform-specific header files
AC_CHECK_HEADERS(sys/fdio.h)

dnl Check for types
AC_SYS_LARGEFILE
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINTPTR_T
AC_TYPE_SSIZE_T
AC_TYPE_SIZE_T
AC_TYPE_OFF_T

AC_TYPE_UID_T
AC_TYPE_PID_T

AC_CHECK_TYPES(long long)
AC_CHECK_SIZEOF(size_t)
AC_CHECK_SIZEOF(off_t)
AC_CHECK_SIZEOF(time_t)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(int)

seek_function=

if test "0$ac_cv_sizeof_off_t" -ge 8 ; then
   seek_function=lseek
fi

dnl Fallback if we have no suitable 64 bit seek function yet
if test X$seek_function = X ; then
  AC_DEFINE([_LARGEFILE64_SOURCE],1,[Needed for off64_t / lseek64 ])
  AC_CHECK_TYPES(off64_t,
    [ AC_CHECK_FUNCS(lseek64, [ seek_function=lseek64 ])
    ])
  AC_CHECK_FUNCS(stat64)
  if test X$seek_function = Xlseek64 ; then
    AC_MSG_CHECKING(whether lseek64 declared in unistd.h)
    AC_CACHE_VAL(mtools_cv_have_lseek64_prototype,
      AC_COMPILE_IFELSE(
         [AC_LANG_PROGRAM([[
              #define _LARGEFILE64_SOURCE
              #include <sys/types.h>
              #include <unistd.h>
            ]], [[extern int lseek64(int);]])],
        [mtools_cv_have_lseek64_prototype=no],
        [mtools_cv_have_lseek64_prototype=yes]))
    AC_MSG_RESULT($mtools_cv_have_lseek64_prototype)
    if test "$mtools_cv_have_lseek64_prototype" = yes; then
      AC_DEFINE([HAVE_LSEEK64_PROTOTYPE],1,[Define when you have an LSEEK64 prototype])
    fi
  fi
fi

dnl Fallback if we have no suitable 64 bit seek function yet
if test X$seek_function = X ; then
  AC_DEFINE([_LARGEFILE_SOURCE],1,[Might be needed for loff_t / llseek64 ])
  AC_CHECK_TYPES([loff_t, offset_t, long long],
    [ AC_CHECK_FUNCS(llseek, [ seek_function=llseek ])
    ])

  if test X$seek_function = Xllseek ; then
    dnl
    dnl Check to see if llseek() is declared in unistd.h.  On some libc's
    dnl it is, and on others it isn't..... Thank you glibc developers....
    dnl
    dnl Warning!  Use of --enable-gcc-wall may throw off this test.
    dnl
    dnl
    AC_MSG_CHECKING(whether llseek declared in unistd.h)
    AC_CACHE_VAL(mtools_cv_have_llseek_prototype,
      AC_COMPILE_IFELSE(
         [AC_LANG_PROGRAM([[
              #define _LARGEFILE_SOURCE
              #define _LARGEFILE64_SOURCE
              #include <sys/types.h>
              #include <unistd.h>
	   ]], [[extern int llseek(int);]])],
        [mtools_cv_have_llseek_prototype=no],
        [mtools_cv_have_llseek_prototype=yes]))
    AC_MSG_RESULT($mtools_cv_have_llseek_prototype)
    if test "$mtools_cv_have_llseek_prototype" = yes; then
      AC_DEFINE([HAVE_LLSEEK_PROTOTYPE],1,[Define when you have an LLSEEK prototype])
    fi
  fi
fi

AC_CHECK_FUNCS(htons)

dnl Apparently termio before termios is preferred by A/UX, AIX and SCO

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

AC_STRUCT_TM


dnl Checks for library functions.

AC_CHECK_FUNCS(strerror lrand48 srand48 strchr strrchr lockf flock \
strcasecmp strncasecmp strnlen atexit on_exit getpass memmove \
strdup strndup strcspn strspn strtoul strtol strtoll strtoui \
memcpy strpbrk memset setenv seteuid setresuid \
tcsetattr tcflush basename  \
readdir snprintf setlocale strstr toupper_l strncasecmp_l \
wcsdup wcscasecmp wcsnlen wcspbrk putwc \
alarm sigaction usleep lstat unsetenv mkdir)


AC_CHECK_FUNCS(utimes utime, [break])
AC_CHECK_FUNCS(tzset gettimeofday)

AC_CHECK_DECLS([sys_errlist, errno],,,[AC_INCLUDES_DEFAULT
#include <errno.h>])
AC_CHECK_DECLS([optarg])

[
echo "Host OS=$host_os"
host_os0=`echo $host_os | sed 's/-/_/g'`
host_os1=`echo "$host_os0" | sed 's/\./_/g'`
host_os2=`echo "$host_os0" | sed 's/^\([^.]*\)\..*$/\1/g'`
host_os3=`echo "$host_os2" | sed 's/^\([^0-9]*\)[0-9]*$/\1/g'`
host_cpu1=`echo $host_cpu | sed 's/\./_/g'`
host_vendor1=`echo $host_vendor | sed 's/\./_/g'`
HOST_ID="-DCPU_$host_cpu1 -DVENDOR_$host_vendor1 -DOS_$host_os1"
if [ "$host_os1" != "$host_os2" ] ; then
	HOST_ID="$HOST_ID -DOS_$host_os2"
fi
if [ "$host_os1" != "$host_os3" ] && [ "$host_os2" != "$host_os3" ] ; then
	HOST_ID="$HOST_ID -DOS_$host_os3"
fi

my_host_os=`echo "$host_os1" "$host_os2" "$host_os3" | sort -u`
if [ "X$GCC" = "Xyes" ] ; then
    if [ "$host_os3" = aux ] ; then
	    CFLAGS="$CFLAGS -ZP"
	    MACHDEPLIBS="-lposix -UTIL"
    fi
    case "${host}" in
       arm*-*-linux) CFLAGS="$CFLAGS -mstructure-size-boundary=8";;
    esac
else
    if [ "$host_os3" = hpux ] ; then
	    CPPFLAGS="$CPPFLAGS -Ae"
    fi

    if [ "$host_os3" = xenix ] ; then
	    CFLAGS="$CFLAGS -M2e"
    fi
fi

if [ "$host_os3" = hpux ] ; then
	    LDFLAGS="$LDFLAGS -z"
fi

if [ "$host_os3" = xenix ] ; then
    LDFLAGS="$LDFLAGS -M2e -i -f 5000"
fi

if [ "$host_os2" = sysv4 ] ; then
    SHLIB="-lc -L/usr/ucblib -lucb"
else
    SHLIB=""
fi

if [ "$host_os3" = isc ] ; then
    CFLAGS="$CFLAGS -D_SYSV3"
    SHLIB="-lc_s"
fi

if [ "$host_os3" = solaris -a x$newVold = xxyes ] ; then
    SHLIB="$SHLIB -s -lvolmgt"
fi

if [ "$host_os3" = nextstep ] ; then
    CFLAGS="$CFLAGS -DBSD"
    SHLIB=""
fi

if [ -d /usr/5lib ] ; then
	extralibdir=-L/usr/5lib
fi

]

AC_PATH_X

dnl Floppyd
AC_ARG_ENABLE(floppyd,
[  --enable-floppyd       floppy daemon support],
[if test x$enableval != x; then
  use_floppyd=$enableval
fi])

if test X$use_floppyd = X -a X$no_x = X ; then
    use_floppyd="yes"
fi

if test X$use_floppyd = Xyes; then
    if test X$no_x = Xyes ; then
	echo "Floppyd needs X support" >&2
	echo "To compile without floppyd, use ./configure --disable-floppyd" >&2
	exit 1
    fi
    FLOPPYD="floppyd floppyd_installtest"
    BINFLOPPYD="\$(DESTDIR)\$(bindir)/floppyd \$(DESTDIR)\$(bindir)/floppyd_installtest"
    FLOPPYD_IO_SRC=floppyd_io.c
    FLOPPYD_IO_OBJ=floppyd_io.o
    AC_DEFINE([USE_FLOPPYD],1,[Define when you want to include floppyd support])
    AC_CHECK_FUNCS(setpgrp getuserid getgroupid)
    AC_FUNC_SETPGRP
    FLOPPYD_LIBS="-lX11"
    AC_PATH_XTRA
    AC_CHECK_HEADERS(sys/socket.h arpa/inet.h netdb.h netinet/in.h \
                     netinet/tcp.h X11/Xlib.h)
else
    FLOPPYD=
    BINFLOPPYD=
    FLOPPYD_IO_SRC=
    FLOPPYD_IO_OBJ=
    FLOPPYD_LIBS=
fi



AC_SUBST(FLOPPYD_LIBS)
AC_SUBST(FLOPPYD)
AC_SUBST(BINFLOPPYD)
AC_SUBST(FLOPPYD_IO_SRC)
AC_SUBST(FLOPPYD_IO_OBJ)

AC_SUBST(extraincludedir)
AC_SUBST(extralibdir)
AC_SUBST(MACHDEPLIBS)
AC_SUBST(SHLIB)
AC_SUBST(host_cpu)
AC_SUBST(HOST_ID)
AC_CONFIG_FILES([Makefile])
AC_CONFIG_HEADERS([config.h])
AC_OUTPUT

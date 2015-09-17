AC_INIT([monitord], m4_esyscmd(git describe --abbrev=6 --dirty --always --tags | tr -d '\n\r'), [monitor@lists.jhr-online.de])
AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

m4_pattern_allow([AM_PROG_AR])
AM_PROG_AR

AC_PROG_CC
AC_PROG_CXX
#AC_C_CONST
#AC_PROG_RANLIB

AX_LUA_HEADERS
AX_LUA_LIBS

case "$host_os" in
	linux*)
		LINUX=true
		WINDOWS=false
		LAMELIBNAME=[mp3lame]
		LAMELIBDLL=[libmp3lame.so] 
		VORBISLIBDLL=[libvorbis.so] 		
		VORBISLIBNAME=[vorbis]
		VORBISENCLIBDLL=[libvorbisenc.so] 		
		VORBISENCLIBNAME=[vorbisenc]
		SOXLIBNAME=[sox]
		SOXLIBDLL=[libsox.so] 	
		MYSQLLIBNAME=[mysqlclient]
		MYSQLLIBDLL=[libmysqlclient.so] 
		ACTIVEMQLIBNAME=[activemq-cpp]
		ACTIVEMQDLL=[libactivemq-cpp.so]
		;;
	mingw32*)
		WINDOWS=true
		LINUX=false
		LAMELIBNAME=[mp3lame-0]
		LAMELIBDLL=[libmp3lame-0.dll] 	
		SOXLIBNAME=[sox-1]
		SOXLIBDLL=[libsox-1.dll] 	
		VORBISLIBDLL=[libvorbis-0.dll] 		
		VORBISLIBNAME=[vorbis-0]
		VORBISENCLIBDLL=[libvorbisenc-2.dll] 		
		VORBISENCLIBNAME=[vorbisenc-0]
		MYSQLLIBNAME=[mysql]
		MYSQLLIBDLL=[libmysql.dll]
		ACTIVEMQNAME=]activemq-cpp]
		ACTIVEMQDLL=[libactivemq-cpp.dll]
		AC_LIBTOOL_WIN32_DLL
		;;
esac

if test x$WINDOWS != xtrue; then
	AC_ARG_WITH([alsa],
		[AC_HELP_STRING([--with-alsa],
		[enable ALSA support (default is yes)])],
		[use_alsa=$withval],
		[use_alsa=yes])
	AS_IF([test "x$use_alsa" != xno],
		[AM_PATH_ALSA(0.9.1, HAVE_ALSA=yes, HAVE_ALSA=no)])
fi
AM_CONDITIONAL(ALSA, test x$HAVE_ALSA = xyes)

AC_ARG_WITH([log4cxx],
	[AC_HELP_STRING([--with-log4cxx],
		[enable log4cxx support (default is no)])],
	[
		(use_log4cxx=$withval)
		HAVE_LOG4CXX=yes
		AC_DEFINE([HAVE_LOG4CXX], [1], [defines that you have log4cxx])
	],
	[use_log4cxx=no])

AM_CONDITIONAL(LOG4CXX, test x$HAVE_LOG4CXX = xyes)

AC_ARG_WITH([log4cxx-includes],
	AC_HELP_STRING([--with-log4cxx-includes=PFX], [Prefix where log4cxx header files are]),
	[gccLog4cxxIncludes="$withval"
		AC_SUBST(LOG4CXX_INCLUDE, "$gccLog4cxxIncludes")],)

AC_ARG_WITH([log4cxx-libraries],
	AC_HELP_STRING([--with-log4cxx-libraries=PFX], [Prefix where where libraries for log4cxx are. In the most environments, this must be set]),
	[gccLog4cxxLibraries="$withval"
		AC_SUBST(LOG4CXX_LIBS, "$gccLog4cxxLibraries")],)
		
if test x$WINDOWS != xtrue; then
	# Linux OS:
	# First check for libm - otherwise libmp3lame will always fail
	# Since we need libm this is no restriction
	AC_CHECK_LIB(m, cos)
fi

AC_ARG_ENABLE([plugins],
	[AC_HELP_STRING([--enable-plugins], 
	[enable experimental plugin support (default is no)])],
	[plugins=yes],
	[plugins=no])
AC_ARG_WITH([mysql],
	[AC_HELP_STRING([--with-mysql],
	[enable experimental mysql support (default is no)])],
	[],
	[use_mysql=no])
AC_ARG_WITH([activemq],
	[AC_HELP_STRING([--with-activemq],
	[enable experimental ActiveMQ support (default is no)])],
	[use_als=$withval],
	[use_activemq=no])
AC_ARG_WITH([lame],
	[AC_HELP_STRING([--with-lame],
	[enable experimental mp3 lame support (default is no)])],
	[],
	[use_lame=no])
AC_ARG_WITH([sox],
	[AC_HELP_STRING([--with-sox],
	[enable experimental libsox support (default is no)])],
	[use_sox=$withval],
	[use_sox=no])
AC_ARG_WITH([libvorbis],
	[AC_HELP_STRING([--with-libvorbis],
	[enable experimental ogg vorbis support (default is no)])],
	[use_vorbis=$withval],
	[use_vorbis=no])
	
if test x$plugins == xno; then
	lame=false
	mysql=false
	if test "x$use_mysql" != xno; then
		AC_MSG_ERROR([[mysql support is not supported without --enable-plugins]])
	fi
	
	if test "x$use_lame" != xno; then
		AC_MSG_ERROR([[lame support is not supported without --enable-plugins]])
	fi

	if test "x$use_sox" != xno; then
		AC_MSG_ERROR([[sox support is not supported without --enable-plugins]])
	fi
	
	if test "x$use_vorbis" != xno; then
		AC_MSG_ERROR([[vorbis support is not supported without --enable-plugins]])
	fi
	if test "x$use_activemq" != xno; then
		AC_MSG_ERROR([[ActiveMQ support is not supported without --enable-plugins]])
	fi
else
	if test "x$use_mysql" != xno; then
	 AC_CHECK_HEADER([mysql/mysql.h],
                     ,
                     [AC_MSG_ERROR([mysql.h not found])])
	
	fi
		
	LIBMYSQLCLIENT=
		AS_IF([test "x$use_mysql" != xno] ,
			[AC_CHECK_LIB([$MYSQLLIBNAME],[mysql_init],
				[AC_SUBST([LIBMYSQLCLIENT], ["-l$MYSQLLIBNAME"])
				AC_DEFINE([HAVE_LIBMYSQLCLIENT], [1],
						[Define if you have libmysqlclient])
				mysql=true
				],
				[if test "x$use_mysql" != xcheck; then
					mysql=true
					AC_MSG_FAILURE(
						[--with-mysql was given, but test for $MYSQLLIBNAME failed])
				fi
			])
		])
	
	LIBMP3LAME=
		AS_IF([test "x$use_lame" != xno],
			[AC_CHECK_LIB([$LAMELIBNAME],[lame_init],
				[AC_SUBST([LIBMP3LAME], ["-l$LAMELIBNAME"])
				AC_DEFINE([HAVE_LIBMP3LAME], [1],
						[Define if you have libmp3lame])
					lame=true
					#[monitord_postdep_objects]=[$LAMELIBDLL]
				],
				[if test "x$use_lame" != xcheck; then
					AC_MSG_FAILURE(
						[--with-lame was given, but test for $LAMELIBNAME failed])
				fi
			])
		])

	if test "x$use_sox" != xno; then
		AC_CHECK_HEADER([sox.h],
		,
		[AC_MSG_ERROR([sox not found])])
	fi
	
	AS_IF([test "x$use_activemq" != xno],
			[AC_CHECK_LIB([$ACTIVEMQLIBNAME],[main],
				[
					AC_SUBST([LIBACTIVEMQ], ["-l$ACTIVEMQLIBNAME"])
					AC_DEFINE([HAVE_ACTIVEMQ], [1], [Define if you have activemq])
					activemq=true
				],
				[if test "x$use_activemq" != xcheck; then
					AC_MSG_FAILURE([--with-activemq but $ACTIVEMQLIBNAME failed])
				fi
			])
		])


	LIBS=
		AS_IF([test "x$use_sox" != xno],
			[AC_CHECK_LIB([$SOXLIBNAME],[sox_init],
				[AC_SUBST([LIBSOX], ["-l$SOXLIBNAME"])
				AC_DEFINE([HAVE_LIBSOX], [1],
						[Define if you have libsox])
					sox=true
					#[monitord_postdep_objects]=[$SOXLIBDLL]
				],
				[if test "x$use_lame" != xcheck; then
					AC_MSG_FAILURE(
						[--with-sox was given, but test for $SOXLIBNAME failed])
				fi
			])
		])

		
	LIBVORBIS=
		AS_IF([test "x$use_vorbis" != xno],
			[AC_CHECK_LIB([$VORBISLIBNAME],[vorbis_block_init],
				[AC_SUBST([LIBVORBIS], ["-l$VORBISLIBNAME -$VORBISENCLIBDLL"])
				AC_DEFINE([HAVE_LIBVORBIS], [1],
						[Define if you have libvorbis])
					vorbis=true
					#[monitord_postdep_objects]=[$VORBISLIBDLL]
				],
				[if test "x$use_vorbis" != xcheck; then
					AC_MSG_FAILURE(
						[--with-vorbis was given, but test for $VORBISLIBNAME failed])
				fi
			])
		])
		
fi

AM_CONDITIONAL(MYSQLSTORAGE, test x$mysql = xtrue)
AM_CONDITIONAL(LINUX, test x$LINUX = xtrue)
AM_CONDITIONAL(WINDOWS, test x$WINDOWS = xtrue)
AM_CONDITIONAL(PLUGINS, test x$plugins = xyes)
AM_CONDITIONAL(LAME, test x$lame = xtrue)
AM_CONDITIONAL(SOX, test x$sox = xtrue)
AM_CONDITIONAL(VORBIS, test x$vorbis = xtrue)
AM_CONDITIONAL(ACTIVEMQ, test x$activemq = xtrue)

AC_SUBST(LUA_LIB)
AC_SUBST(LUA_INCLUDE)

if test x$WINDOWS = xtrue; then
	AC_CHECK_TOOL(WINDRES, windres)
fi

AC_PROG_LIBTOOL
#AM_PROG_LIBTOOL

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
	Makefile
	win32-nsis/monitor.nsi
])
AC_OUTPUT

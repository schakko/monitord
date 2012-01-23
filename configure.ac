AC_INIT([monitord], [2.0svn], [monitor@lists.jhr-online.de])
AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE([-Wall -Werror foreign])


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
                     [AC_MSG_ERROR([sox.h not found])])
	fi

	LIBSOX=
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

# Tell automake to put the object file for monitord/apple.c in dir monitord/
AUTOMAKE_OPTIONS := subdir-objects
ACLOCAL_AMFLAGS = -I m4

# The installable executable.
bin_PROGRAMS = monitord/monitord

#=========================================================
# monitord : The application.

monitord_monitord_SOURCES = \
	monitord/Monitor.cpp \
	monitord/MonitorAudio.cpp \
	monitord/MonitorConfiguration.cpp \
	monitord/MonitorModuleFMS.cpp \
	monitord/MonitorModulePocsag.cpp \
	monitord/MonitorModulePocsag1200.cpp \
	monitord/MonitorModulePocsag512.cpp \
	monitord/MonitorModuleZVEI.cpp \
	monitord/MonitorModules.cpp \
	monitord/MonitorModulesResults.cpp \
	monitord/SndPipe.cpp \
	monitord/SocketServer.cpp \
	monitord/SocketThreadMonitord.cpp \
	monitord/SocketThreadFMS32.cpp \
	monitord/SocketThreadCrusader.cpp \
	monitord/memlock.cpp \
	monitord/xmltools.cpp \
	monitord/MonitorSignals.cpp \
	xmlParser/xmlParser.cpp


EXTRA_DIST =\
	xmlParser/xmlParser.h \
	monitord/*.h \
	monitord/plugins/*.h \
	monitord/generators/*.h \
	monitord/posix/*.h \
	monitord/win32/*.h \
	jthread-1.2.1/src/*.h \
	simpleopt/*.h \
	monitord/lua.hpp \
	sample-config/*xml*

monitord_monitord_CPPFLAGS = -Ijthread-1.2.1/src -D_DEBUG -Wall 
# ausgeschaltet -pedantic
monitord_monitord_LDFLAGS = -g
monitord_monitord_LDADD =


if LINUX
monitord_monitord_SOURCES += \
	jthread-1.2.1/src/pthread/jmutex.cpp \
	jthread-1.2.1/src/pthread/jthread.cpp
monitord_monitord_LDFLAGS += -lpthread

if ALSA
monitord_monitord_SOURCES += \
	monitord/posix/MonitorAudioALSA.cpp
monitord_monitord_LDFLAGS += @ALSA_LIBS@
else
monitord_monitord_SOURCES += \
	monitord/posix/MonitorAudioOSS.cpp
endif
endif

# LUA Headers und Libs einbinden
monitord_monitord_CPPFLAGS += @LUA_INCLUDE@
monitord_monitord_LDFLAGS += @LUA_LIB@


if WINDOWS
monitord_monitord_SOURCES += \
	jthread-1.2.1/src/win32/jmutex.cpp \
	jthread-1.2.1/src/win32/jthread.cpp \
	monitord/win32/MonitorAudioWin32.cpp \
	monitord/win32/MonitorService.cpp
monitord_monitord_CPPFLAGS += -I${PWD}
monitord_monitord_LDADD += -lwinmm -lws2_32
monitord_monitord_LDFLAGS += -Wl,--subsystem,console -mconsole

.rc.o:
	@WINDRES@ -o $@ $<
endif


if PLUGINS
monitord_monitord_CPPFLAGS += -frtti -DPLUGINS
monitord_monitord_SOURCES += \
	monitord/PluginThread.cpp \
	monitord/plugins/mplugin.cpp \
	monitord/plugins/mpluginAudio.cpp

if LINUX
monitord_monitord_CPPFLAGS += -fPIC -rdynamic
monitord_monitord_LDFLAGS += -ldl
monitord_monitord_SOURCES += \
	monitord/plugins/dll-elf.cpp
DLL = -shared -frtti
endif

if WINDOWS
monitord_monitord_CPPFLAGS += -DWIN32
monitord_monitord_LDFLAGS += -lkernel32 -lstdc++
DLL = -shared  -Wl,--export-dynamic -Wl,--output-def=cxxdll.def -Wl,--enable-auto-image-base
monitord_monitord_SOURCES += \
	monitord/plugins/dll-win32.cpp
endif


if SOX

pkglib_LTLIBRARIES= \
	monitord/plugins/libmplugin_audiorecorderSOX.la

else
pkglib_LTLIBRARIES= \
	monitord/plugins/libmplugin_audiorecorder.la

endif

monitord_plugins_common= \
	monitord/xmltools.cpp \
	xmlParser/xmlParser.cpp
# --------------------------------------------------------
# Monitor-Plugin: MySQL
#

if MYSQLSTORAGE
pkglib_LTLIBRARIES+= \
	monitord/plugins/libmplugin_mysql.la

monitord_plugins_libmplugin_mysql_la_SOURCES=\
	${monitord_plugins_common} \
	monitord/plugins/mplugin.cpp \
	monitord/plugins/libmplugin_mysql.cpp

monitord_plugins_libmplugin_mysql_la_CPPFLAGS =\
	${monitord_monitord_CPPFLAGS} \
	-DMYSQLSTORAGE \
	${DLL}

monitord_plugins_libmplugin_mysql_la_LDFLAGS=\
	-no-undefined \
	${LIBMYSQLCLIENT} \
	${DLL}

monitord_plugins_libmplugin_mysql_la_LIBS=\
	${LIBMYSQLCLIENT}

endif # mysql Support ?

# --------------------------------------------------------
# Audio-Plugin: Recorder
#
monitord_plugins_libmplugin_audiorecorder_la_SOURCES=\
	${monitord_plugins_common} \
	monitord/plugins/mplugin.cpp \
	monitord/plugins/mpluginAudio.cpp \
	monitord/plugins/libmplugin_audiorecorder.cpp

monitord_plugins_libmplugin_audiorecorder_la_CPPFLAGS =\
	${monitord_monitord_CPPFLAGS} \
	${DLL}

monitord_plugins_libmplugin_audiorecorder_la_LDFLAGS=\
	-no-undefined \
	${DLL}

if LAME
monitord_plugins_libmplugin_audiorecorder_la_LDFLAGS+=\
	${LIBMP3LAME}

if LINUX
monitord_plugins_libmplugin_audiorecorder_la_LDFLAGS+=\
	-lm

endif # LAME && LINUX

endif  # LAME

if SOX
# --------------------------------------------------------
# Audio-Plugin: Recorder (SOX)
#
monitord_plugins_libmplugin_audiorecorderSOX_la_SOURCES=\
	${monitord_plugins_common} \
	monitord/plugins/mplugin.cpp \
	monitord/plugins/mpluginAudio.cpp \
	monitord/plugins/libmplugin_audiorecorderSOX.cpp

monitord_plugins_libmplugin_audiorecorderSOX_la_CPPFLAGS =\
	${monitord_monitord_CPPFLAGS} \
	${DLL}

monitord_plugins_libmplugin_audiorecorderSOX_la_LDFLAGS=\
	-no-undefined \
	${LIBSOX} \
	${DLL}


if LINUX
monitord_plugins_libmplugin_audiorecorderSOX_la_LDFLAGS+=\
	-lm
endif # SOX && LINUX

endif  # SOX

endif # PLUGINS


#=========================================================

dok:
	(cd monitord && doxygen doxygen.config)

if WINDOWS
nightly: @PACKAGE_TARNAME@-@host_os@-@PACKAGE_VERSION@.exe @PACKAGE_TARNAME@-@host_os@-@PACKAGE_VERSION@.zip

@PACKAGE_TARNAME@-@host_os@-@PACKAGE_VERSION@.exe: monitord/monitord.exe win32-nsis/monitor.nsi
	makensis -NOCD win32-nsis/monitor.nsi

@PACKAGE_TARNAME@-@host_os@-@PACKAGE_VERSION@.zip: monitord/monitord.exe
	if [ -d @PACKAGE_TARNAME@-@host_os@-@PACKAGE_VERSION@ ]; then rm -rf @PACKAGE_TARNAME@-@host_os@-@PACKAGE_VERSION@; fi
	mkdir @PACKAGE_TARNAME@-@host_os@-@PACKAGE_VERSION@
	cp monitord/monitord.exe @PACKAGE_TARNAME@-@host_os@-@PACKAGE_VERSION@
	cp sample-config/monitord.xml.win32 @PACKAGE_TARNAME@-@host_os@-@PACKAGE_VERSION@/monitord.xml
	zip -r9 @PACKAGE_TARNAME@-@host_os@-@PACKAGE_VERSION@.zip @PACKAGE_TARNAME@-@host_os@-@PACKAGE_VERSION@
	rm -rf @PACKAGE_TARNAME@-@host_os@-@PACKAGE_VERSION@
endif # WINDOWS


### all of the standard pc files we need to generate
pcfiles = 							\
	jthread.pc

pcfiles_uninstalled =						\
	jthread-uninstalled.pc

all-local: $(pcfiles) $(pcfiles_uninstalled)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(pcfiles)

EXTRA_DIST =							\
	jthread.pc.in						\
	jthread-uninstalled.pc.in

CLEANFILES = $(pcfiles) $(pcfiles_uninstalled)

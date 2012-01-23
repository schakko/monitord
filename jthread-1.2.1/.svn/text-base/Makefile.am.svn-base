SUBDIRS = src doc pkgconfig
EXTRA_DIST = LICENSE.MIT ChangeLog README.TXT jthread.dsw jthread.dsp jthread.vcproj jthread.sln \
	     jthread_wce.sln jthread_wce.vcproj jthread_wce5.sln jthread_wce5.vcproj

dist-hook: distclean
	mv $(distdir)/jthread.dsw $(distdir)/jthread.dsw.tmp
	mv $(distdir)/jthread.dsp $(distdir)/jthread.dsp.tmp
	todos < $(distdir)/jthread.dsw.tmp > $(distdir)/jthread.dsw
	todos < $(distdir)/jthread.dsp.tmp > $(distdir)/jthread.dsp
	rm $(distdir)/jthread.dsw.tmp
	rm $(distdir)/jthread.dsp.tmp
	chmod a-x $(distdir)/jthread_wce.vcproj
	chmod a-x $(distdir)/jthread_wce.sln
	chmod a-x $(distdir)/jthread.vcproj
	chmod a-x $(distdir)/jthread.sln


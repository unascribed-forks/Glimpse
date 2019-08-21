## Makefile.am for gimp/desktop

desktop_in_files = \
	app.getglimpse.Glimpse.desktop.in.in

desktop_files = $(desktop_in_files:.desktop.in.in=.desktop)

appstream_in_files = \
	app.getglimpse.Glimpse.appdata.xml.in	\
	glimpse-data-extras.metainfo.xml.in

appstream_files = $(appstream_in_files:.xml.in=.xml)

if DESKTOP_DATADIR

applicationsdir = $(DESKTOP_DATADIR)/applications
applications_DATA = $(desktop_files)

appdatadir = $(DESKTOP_DATADIR)/metainfo
appdata_DATA = $(appstream_files)

icons16dir = $(DESKTOP_DATADIR)/icons/hicolor/16x16/apps
icons16_DATA = 16x16/app.getglimpse.Glimpse.png

icons22dir = $(DESKTOP_DATADIR)/icons/hicolor/22x22/apps
icons22_DATA = 22x22/app.getglimpse.Glimpse.png

icons24dir = $(DESKTOP_DATADIR)/icons/hicolor/24x24/apps
icons24_DATA = 24x24/app.getglimpse.Glimpse.png

icons32dir = $(DESKTOP_DATADIR)/icons/hicolor/32x32/apps
icons32_DATA = 32x32/app.getglimpse.Glimpse.png

icons48dir = $(DESKTOP_DATADIR)/icons/hicolor/48x48/apps
icons48_DATA = 48x48/app.getglimpse.Glimpse.png

icons256dir = $(DESKTOP_DATADIR)/icons/hicolor/256x256/apps
icons256_DATA = 256x256/app.getglimpse.Glimpse.png


# touch the toplevel icon directory as required by the icon theme spec
install-data-hook:
	touch $(DESTDIR)$(DESKTOP_DATADIR)/icons/hicolor

endif

EXTRA_DIST = \
	$(appstream_in_files) \
	test-appdata.sh.in    \
	test-desktop.sh       \
	app.getglimpse.Glimpse.desktop.in.in    \
	16x16/app.getglimpse.Glimpse.png        \
	22x22/app.getglimpse.Glimpse.png        \
	24x24/app.getglimpse.Glimpse.png        \
	32x32/app.getglimpse.Glimpse.png        \
	48x48/app.getglimpse.Glimpse.png        \
	64x64/app.getglimpse.Glimpse.png        \
	256x256/app.getglimpse.Glimpse.png


DISTCLEANFILES = $(desktop_files) $(appstream_files)

@INTLTOOL_XML_RULE@

app.getglimpse.Glimpse.desktop: app.getglimpse.Glimpse.desktop.in $(wildcard $(top_srcdir)/po/*.po)
	$(INTLTOOL_MERGE) $(top_srcdir)/po $< $(@) -d -u -c $(top_builddir)/po/.intltool-merge-cache

validate: app.getglimpse.Glimpse.desktop
	( dfvalidate=`which desktop-file-validate`; \
	  if test x$$dfvalidate != x && test -x $$dfvalidate; then \
            $$dfvalidate $< || ( echo "* $< INVALID *"; exit 1 ) \
	  else \
	    echo "Can't find desktop-file-validate to validate $<; proceed with fingers crossed..."; \
          fi )

dist-hook: validate

TESTS_ENVIRONMENT = \
	chmod +x test-appdata.sh; \
	GIMP_TESTING_ABS_TOP_BUILDDIR=@abs_top_builddir@
TESTS =

if HAVE_APPSTREAM_UTIL
TESTS += test-appdata.sh
endif

if HAVE_DESKTOP_FILE_VALIDATE
TESTS += test-desktop.sh
endif

## Process this file with automake to produce Makefile.in
## $Id: $

EXTRA_DIST = \
  fuppes.server.in

if GNOMEAPPLET

pixmapsdir = $(datadir)/pixmaps

INCLUDES = \
	${LIBPANELAPPLET_CFLAGS} \
	${LIBNOTIFY_CFLAGS} \
	-DFUPPES_PIXMAPSDIR=\"$(pixmapsdir)\"

libexec_PROGRAMS = fuppes-applet

fuppes_applet_SOURCES = \
	fuppes-applet.c \
  fuppes-applet.h

fuppes_applet_LDADD = ./../libfuppes.la

fuppes_applet_LDFLAGS = \
  $(LIBPANELAPPLET_LIBS) \
  ${LIBNOTIFY_LIBS}

dist_pixmaps_DATA = ./../../fuppes.svg

serverdir = $(libdir)/bonobo/servers
server_SCRIPTS = fuppes.server
CLEANFILES = $(server_SCRIPTS)

# create fuppes.server
fuppes.server: fuppes.server.in
	sed \
  -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
  -e "s|\@PIXMAPSDIR\@|$(pixmapsdir)|" \
  $< > $@

endif

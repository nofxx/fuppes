## Process this file with automake to produce Makefile.in
## $Id: $

if GNOMEAPPLET

INCLUDES = ${LIBPANELAPPLET_CFLAGS}

libexec_PROGRAMS = fuppes-applet

fuppes_applet_SOURCES = \
  fuppes-applet.c\
  fuppes-applet.h

EXTRA_DIST = fuppes-applet.c.in
pixmapsdir = $(datadir)/pixmaps
dist_pixmaps_DATA = ./../../fuppes.svg

fuppes-applet.c: fuppes-applet.c.in
	sed \
  -e "s|\@PIXMAPSDIR\@|$(pixmapsdir)|" \
  $< > $@

fuppes_applet_LDADD = ./../libfuppes.la

fuppes_applet_LDFLAGS = \
  $(LIBPANELAPPLET_LIBS)


serverdir = $(libdir)/bonobo/servers
server_SCRIPTS = fuppes.server
CLEANFILES = $(server_SCRIPTS)
EXTRA_DIST = fuppes.server.in

fuppes.server: fuppes.server.in
	sed \
  -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
  -e "s|\@PIXMAPSDIR\@|$(pixmapsdir)|" \
  $< > $@

endif

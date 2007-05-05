## Process this file with automake to produce Makefile.in
## $Id: $

if GNOMEAPPLET

INCLUDES = ${LIBPANELAPPLET_CFLAGS} ${LIBNOTIFY_CFLAGS}

libexec_PROGRAMS = fuppes-applet

fuppes_applet_SOURCES = \
  fuppes-applet.c\
  fuppes-applet.h

fuppes_applet_LDADD = ./../libfuppes.la

fuppes_applet_LDFLAGS = \
  $(LIBPANELAPPLET_LIBS) \
  ${LIBNOTIFY_LIBS}

EXTRA_DIST = fuppes-applet.c.in \
  fuppes.server.in

# directories
pixmapsdir = $(datadir)/pixmaps
dist_pixmaps_DATA = ./../../fuppes.svg

serverdir = $(libdir)/bonobo/servers
server_SCRIPTS = fuppes.server
CLEANFILES = $(server_SCRIPTS) fuppes-applet.c


# create fuppes-applet.c
fuppes-applet.c: fuppes-applet.c.in
	sed \
  -e "s|\@PIXMAPSDIR\@|$(pixmapsdir)|" \
  $< > $@

# create fuppes.server
fuppes.server: fuppes.server.in
	sed \
  -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
  -e "s|\@PIXMAPSDIR\@|$(pixmapsdir)|" \
  $< > $@

endif

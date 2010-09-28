
include $(top_srcdir)/PreRules.am

EXTRA_DIST = \
  fuppes_types.h.in

noinst_HEADERS = \
  fuppes.h \
  fuppes_plugin.h \
  fuppes_plugin_types.h \
  fuppes_db_connection_plugin.h

nodist_noinst_HEADERS = \
  fuppes_types.h

CLEANFILES = \
  fuppes_types.h

# create fuppes_types.h
if WIN32
fuppes_off_t = long long int
else
fuppes_off_t = off_t
endif

fuppes_types.h: fuppes_types.h.in
	$(SED) -e "s|\@FUPPES_OFF_T\@|$(fuppes_off_t)|" fuppes_types.h.in > $@


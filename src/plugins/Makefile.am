INCLUDES = \
  ${FUPPES_CFLAGS}\
  ${LIBXML_CFLAGS}\
  ${PCRE_CFLAGS}\
  ${VORBISFILE_CFLAGS}\
  ${TAGLIB_CFLAGS}

libdir = @libdir@/fuppes

lib_LTLIBRARIES =

## libavformat metadata extraction plugin

## if AVFORMAT_METADATA

lib_LTLIBRARIES += libmetadata_libavformat.la

libmetadata_libavformat_la_SOURCES = \
	metadata_libavformat.c

## endif


## taglib metadata extraction plugin

lib_LTLIBRARIES += libmetadata_taglib.la

libmetadata_taglib_la_SOURCES = \
	metadata_taglib.cpp

libmetadata_taglib_la_LDFLAGS = \
	${TAGLIB_LIBS}

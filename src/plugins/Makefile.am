INCLUDES = \
	${LIBAVFORMAT_CFLAGS} \
	${TAGLIB_CFLAGS} \
	${IMAGEMAGICK_WAND_CFLAGS} \
	${SIMAGE_CFLAGS}

libdir = @libdir@/fuppes

lib_LTLIBRARIES =

## libavformat metadata extraction plugin

if LIBAVFORMAT_METADATA

lib_LTLIBRARIES += libmetadata_libavformat.la

libmetadata_libavformat_la_SOURCES = \
	metadata_libavformat.c

libmetadata_libavformat_la_LDFLAGS = \
	${LIBAVFORMAT_LIBS}

endif


## taglib metadata extraction plugin
if TAGLIB_METADATA

lib_LTLIBRARIES += libmetadata_taglib.la

libmetadata_taglib_la_SOURCES = \
	metadata_taglib.cpp

libmetadata_taglib_la_LDFLAGS = \
	${TAGLIB_LIBS}

endif


## simage metadata extraction plugin
if SIMAGE_METADATA

lib_LTLIBRARIES += libmetadata_simage.la

libmetadata_simage_la_SOURCES = \
	metadata_simage.c

libmetadata_simage_la_LDFLAGS = \
	${SIMAGE_LIBS}

endif


## magickWand metadata extraction plugin
if MAGICK_WAND_METADATA

lib_LTLIBRARIES += libmetadata_magickwand.la

libmetadata_magickwand_la_SOURCES = \
	metadata_magickwand.c

libmetadata_magickwand_la_LDFLAGS = \
	${IMAGEMAGICK_WAND_LIBS}

endif

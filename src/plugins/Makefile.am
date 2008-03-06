
libdir = @libdir@/fuppes

lib_LTLIBRARIES =

## libavformat metadata extraction plugin

if LIBAVFORMAT_METADATA

lib_LTLIBRARIES += libmetadata_libavformat.la

libmetadata_libavformat_la_SOURCES = \
	metadata_libavformat.c

libmetadata_libavformat_la_CFLAGS = \
	$(LIBAVFORMAT_CFLAGS)
	
libmetadata_libavformat_la_LDFLAGS = \
	$(LIBAVFORMAT_LIBS)

endif


## taglib metadata extraction plugin
if TAGLIB_METADATA

lib_LTLIBRARIES += libmetadata_taglib.la

libmetadata_taglib_la_SOURCES = \
	metadata_taglib.cpp

libmetadata_taglib_la_CXXFLAGS = \
	$(TAGLIB_CFLAGS)

libmetadata_taglib_la_LDFLAGS = \
	$(TAGLIB_LIBS)

endif


## simage metadata extraction plugin
if SIMAGE_METADATA

lib_LTLIBRARIES += libmetadata_simage.la

libmetadata_simage_la_SOURCES = \
	metadata_simage.c

libmetadata_simage_la_CFLAGS = \
	$(SIMAGE_CFLAGS)
	
libmetadata_simage_la_LDFLAGS = \
	$(SIMAGE_LIBS)

endif


## magickWand metadata extraction plugin
if MAGICK_WAND_METADATA

lib_LTLIBRARIES += libmetadata_magickwand.la

libmetadata_magickwand_la_SOURCES = \
	metadata_magickwand.c

libmetadata_magickwand_la_CFLAGS = \
	$(IMAGEMAGICK_WAND_CFLAGS)
	
libmetadata_magickwand_la_LDFLAGS = \
	$(IMAGEMAGICK_WAND_LIBS)

endif


## ffmpeg transcoder
if VIDEO_TRANSCODING

lib_LTLIBRARIES += libtranscoder_ffmpeg.la

libtranscoder_ffmpeg_la_SOURCES = \
	transcoder_ffmpeg.cpp \
	FFmpegWrapper.cpp \
	FFmpegWrapper.h \
	ffmpeg/cmdutils.cpp \
	ffmpeg/cmdutils.h \
	ffmpeg/ffmpeg.cpp \
	ffmpeg/ffmpeg.h

libtranscoder_ffmpeg_la_CXXFLAGS = \
	$(LIBAVFORMAT_CFLAGS) \
	$(LIBSWSCALE_CFLAGS) \
	-D__STDC_CONSTANT_MACROS
	
libtranscoder_ffmpeg_la_LDFLAGS = \
	$(LIBAVFORMAT_LIBS) \
	$(LIBSWSCALE_LIBS)

endif

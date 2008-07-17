
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

## libmp4v2 metadata extraction plugin
if MP4V2_METADATA

lib_LTLIBRARIES += libmetadata_libmp4v2.la

libmetadata_libmp4v2_la_SOURCES = \
	metadata_libmp4v2.cpp

libmetadata_libmp4v2_la_CXXFLAGS = \
	$(MP4V2_CFLAGS)
	
libmetadata_libmp4v2_la_LDFLAGS = \
	$(MP4V2_LIBS)

endif


## dlna profile detection plugin
if DLNA_SUPPORT

lib_LTLIBRARIES += libmetadata_dlna_profiles.la

libmetadata_dlna_profiles_la_SOURCES = \
	metadata_dlna_profiles.c \
	dlna\dlna_image_profiles.h

libmetadata_dlna_profiles_la_CXXFLAGS = 
	
libmetadata_dlna_profiles_la_LDFLAGS = 

endif



## ffmpeg transcoder
if VIDEO_TRANSCODING

lib_LTLIBRARIES += libtranscoder_ffmpeg.la

libtranscoder_ffmpeg_la_SOURCES = \
	transcoder_ffmpeg.cpp \
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


## Magick++ transcoder
if MAGICK_PP_TRANSCODER

lib_LTLIBRARIES += libtranscoder_magickpp.la

libtranscoder_magickpp_la_SOURCES = \
	transcoder_magickpp.cpp

libtranscoder_magickpp_la_CXXFLAGS = \
	$(IMAGEMAGICK_PP_CFLAGS)
	
libtranscoder_magickpp_la_LDFLAGS = \
	$(IMAGEMAGICK_PP_LIBS)

endif


## L.A.M.E. encoder
#if LAME_ENCODER

#lib_LTLIBRARIES += libencoder_lame.la

#libencoder_lame_la_SOURCES = \
#	encoder_lame.c

#libencoder_lame_la_CFLAGS = \
#	$(LAME_H_DIR)
	
#libencoder_lame_la_LDFLAGS = \
#	$(LAME_LIBS)

#endif
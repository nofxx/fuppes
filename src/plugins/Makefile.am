include $(top_srcdir)/PreRules.am

libdir = @libdir@/fuppes

lib_LTLIBRARIES =

##
##  CORE PLUGINS
##

## basic presentation

lib_LTLIBRARIES += libcore_presentation.la

libcore_presentation_la_SOURCES = \
	core_presentation.cpp


## sqlite3

lib_LTLIBRARIES += libdatabase_sqlite3.la

libdatabase_sqlite3_la_SOURCES = \
	database_sqlite3.cpp

libdatabase_sqlite3_la_CXXFLAGS = \
	$(SQLITE3_CFLAGS)
	
libdatabase_sqlite3_la_LDFLAGS = \
	$(SQLITE3_LIBS)


## MySQL

if MYSQL_DATABASE_PLUGIN
lib_LTLIBRARIES += libdatabase_mysql.la

libdatabase_mysql_la_SOURCES = \
	database_mysql.cpp

libdatabase_mysql_la_CXXFLAGS = \
	$(MYSQL_CFLAGS)
	
libdatabase_mysql_la_LDFLAGS = \
	$(MYSQL_LIBS)
endif






##
##  METADATA PLUGINS
##

## libavformat metadata extraction plugin

if LIBAVFORMAT_METADATA_PLUGIN

lib_LTLIBRARIES += libmetadata_libavformat.la

libmetadata_libavformat_la_SOURCES = \
	metadata_libavformat.c

libmetadata_libavformat_la_CFLAGS = \
	$(LIBAVFORMAT_CFLAGS) \
	$(LIBAVCODEC_CFLAGS)
	
libmetadata_libavformat_la_LDFLAGS = \
	$(LIBAVFORMAT_LIBS) \
	$(LIBAVCODEC_LIBS)

endif


## taglib metadata extraction plugin
if TAGLIB_METADATA_PLUGIN

lib_LTLIBRARIES += libmetadata_taglib.la

libmetadata_taglib_la_SOURCES = \
	metadata_taglib.cpp

libmetadata_taglib_la_CXXFLAGS = \
	$(TAGLIB_CFLAGS)

libmetadata_taglib_la_LDFLAGS = \
	$(TAGLIB_LIBS)

endif


## simage metadata extraction plugin
if SIMAGE_METADATA_PLUGIN

lib_LTLIBRARIES += libmetadata_simage.la

libmetadata_simage_la_SOURCES = \
	metadata_simage.c

libmetadata_simage_la_CFLAGS = \
	$(SIMAGE_CFLAGS)
	
libmetadata_simage_la_LDFLAGS = \
	$(SIMAGE_LIBS)

endif


## magickWand metadata extraction plugin
if MAGICK_WAND_METADATA_PLUGIN

lib_LTLIBRARIES += libmetadata_magickwand.la

libmetadata_magickwand_la_SOURCES = \
	metadata_magickwand.c

libmetadata_magickwand_la_CFLAGS = \
	$(IMAGEMAGICK_WAND_CFLAGS)
	
libmetadata_magickwand_la_LDFLAGS = \
	$(IMAGEMAGICK_WAND_LIBS)

endif

## exiv2 metadata extraction plugin
if EXIV2_METADATA_PLUGIN

lib_LTLIBRARIES += libmetadata_exiv2.la

libmetadata_exiv2_la_SOURCES = \
	metadata_exiv2.cpp

libmetadata_exiv2_la_CXXFLAGS = \
	$(EXIV2_CXXFLAGS)
	
libmetadata_exiv2_la_LDFLAGS = \
	$(EXIV2_LIBS)

endif

## libmp4v2 metadata extraction plugin
if MP4V2_METADATA_PLUGIN

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
	dlna/dlna_image_profiles.h

libmetadata_dlna_profiles_la_CXXFLAGS = 
	
libmetadata_dlna_profiles_la_LDFLAGS = 

endif



##
##  TRANSCODER PLUGINS
##


## ffmpeg transcoder
if FFMPEG_TRANSCODER_PLUGIN

lib_LTLIBRARIES += libtranscoder_ffmpeg.la

libtranscoder_ffmpeg_la_SOURCES = \
	transcoder_ffmpeg.cpp \
	ffmpeg/cmdutils.cpp \
	ffmpeg/cmdutils.h \
	ffmpeg/ffmpeg.cpp \
	ffmpeg/ffmpeg.h

libtranscoder_ffmpeg_la_CXXFLAGS = \
	$(LIBAVFORMAT_CFLAGS) \
	$(LIBAVCODEC_CFLAGS) \
	$(LIBAVUTIL_CFLAGS) \
	$(LIBSWSCALE_CFLAGS) \
	-D__STDC_CONSTANT_MACROS
	
libtranscoder_ffmpeg_la_LDFLAGS = \
	$(LIBAVFORMAT_LIBS) \
	$(LIBAVCODEC_LIBS) \
	$(LIBAVUTIL_LIBS) \
	$(LIBSWSCALE_LIBS)

endif


## MagickWand transcoder
if MAGICK_WAND_TRANSCODER_PLUGIN

lib_LTLIBRARIES += libtranscoder_magickwand.la

libtranscoder_magickwand_la_SOURCES = \
	transcoder_magickwand.c

libtranscoder_magickwand_la_CFLAGS = \
	$(IMAGEMAGICK_WAND_CFLAGS)
	
libtranscoder_magickwand_la_LDFLAGS = \
	$(IMAGEMAGICK_WAND_LIBS)

endif


##
##  AUDIO DECODER PLUGINS
##

## vorbis decoder
if VORBIS_DECODER_PLUGIN

lib_LTLIBRARIES += libdecoder_vorbis.la

libdecoder_vorbis_la_SOURCES = \
	decoder_vorbis.c

libdecoder_vorbis_la_CFLAGS = \
	$(VORBISFILE_CFLAGS)
	
libdecoder_vorbis_la_LDFLAGS = \
	$(VORBISFILE_LIBS)

endif

## FLAC decoder
if FLAC_DECODER_PLUGIN

lib_LTLIBRARIES += libdecoder_flac.la

libdecoder_flac_la_SOURCES = \
	decoder_flac.c

libdecoder_flac_la_CFLAGS = \
	$(FLAC_CFLAGS)
	
libdecoder_flac_la_LDFLAGS = \
	$(FLAC_LIBS)

endif


## MusePack decoder
if MUSEPACK_DECODER_PLUGIN

lib_LTLIBRARIES += libdecoder_musepack.la

libdecoder_musepack_la_SOURCES = \
	decoder_musepack.c

libdecoder_musepack_la_CFLAGS = \
	$(MUSEPACK_CFLAGS)
	
libdecoder_musepack_la_LDFLAGS = \
	$(MUSEPACK_LIBS)

endif

## twolame decoder
#if TWOLAME_ENCODER_PLUGIN

#lib_LTLIBRARIES += libencoder_twolame.la

#libencoder_twolame_la_SOURCES = \
#	encoder_twolame.c

#libencoder_twolame_la_CFLAGS = \
#	$(TWOLAME_CFLAGS)
	
#libencoder_twolame_la_LDFLAGS = \
#	$(TWOLAME_LIBS)

#endif



##
##  AUDIO ENCODER PLUGINS
##

# pcm
lib_LTLIBRARIES += libencoder_pcm.la
libencoder_pcm_la_SOURCES = encoder_pcm.c
libencoder_pcm_la_CFLAGS =	
libencoder_pcm_la_LDFLAGS =

# wav
lib_LTLIBRARIES += libencoder_wav.la
libencoder_wav_la_SOURCES = encoder_wav.c
libencoder_wav_la_CFLAGS =	
libencoder_wav_la_LDFLAGS =

## L.A.M.E. encoder
if LAME_ENCODER_PLUGIN
lib_LTLIBRARIES += libencoder_lame.la
libencoder_lame_la_SOURCES = encoder_lame.c
libencoder_lame_la_CFLAGS = $(LAME_H_DIR)
libencoder_lame_la_LDFLAGS = $(LAME_LIBS)
endif


/***************************************************************************
 *            AudioItem.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 *  Copyright (C) 2005 Thomas Schnitzler <tschnitzler@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#ifndef _AUDIOITEM_H
#define _AUDIOITEM_H

/*===============================================================================
 INCLUDES
===============================================================================*/

#include "UPnPItem.h"

typedef enum tagAUDIO_FORMAT
{
  AUDIO_FORMAT_UNKNOWN  =  0,
  AUDIO_FORMAT_MP3      =  1,
	AUDIO_FORMAT_VORBIS   =  2,
  AUDIO_FORMAT_MPC      =  3,    
  AUDIO_FORMAT_FLAC     =  4
}AUDIO_FORMAT;

typedef enum tagAUDIO_ENCODER
{
  AUDIO_ENCODER_UNKNOWN  =  0,
  AUDIO_ENCODER_NONE     =  1,
  AUDIO_ENCODER_LAME     =  2
}AUDIO_ENCODER;

typedef enum tagAUDIO_DECODER
{
  AUDIO_DECODER_UNKNOWN  =  0,
  AUDIO_DECODER_NONE     =  1,
  AUDIO_DECODER_VORBIS   =  2,
  AUDIO_DECODER_MUSEPACK =  3,
  AUDIO_DECODER_FLAC     =  4
}AUDIO_DECODER;

/*===============================================================================
 CLASS CAudioItem
===============================================================================*/

class CAudioItem: public CUPnPItem
{

/* <PUBLIC> */

  public:

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/
    
    /** constructor
     *  @param  p_sHTTPServerURL  URL of the HTTP server
     */
    CAudioItem(std::string p_sHTTPServerURL);

/*===============================================================================
 GET
===============================================================================*/

    /** writes the whole description of an audio item
     *  @param  pWriter  the XML container to write to
     */
    void GetDescription(xmlTextWriterPtr pWriter);
    
    bool SetupTranscoding();
    bool GetDoTranscode() { return m_bDoTranscode; }
    AUDIO_DECODER GetDecoderType() { return m_nDecoderType; }
  
/* <\PUBLIC> */  
  
  private:
  
    bool           m_bDoTranscode;
    AUDIO_FORMAT   m_nAudioFormat;
    AUDIO_DECODER  m_nDecoderType;
    AUDIO_ENCODER  m_nEncoderType;

};

#endif /* _AUDIOITEM_H */

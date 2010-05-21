#ifndef __TRANSCODING_SETTINGS_H
#define __TRANSCODING_SETTINGS_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <string>
#include "ConfigSettings.h"

class TranscodingSettings : public ConfigSettings
{
  public:
    virtual bool Read(void);

    std::string	 LameLibName() { return m_sLameLibName; }
    std::string  TwoLameLibName() { return m_sTwoLameLibName; }
    std::string  VorbisLibName() { return m_sVorbisLibName; }
    std::string  MpcLibName() { return m_sMpcLibName; }
    std::string  FlacLibName() { return m_sFlacLibName; }
    std::string  FaadLibName() { return m_sFaadLibName; }
		std::string  Mp4ffLibName() { return m_sMp4ffLibName; }
		std::string  MadLibName() { return m_sMadLibName; }

    // transcoding  
    /*std::string  AudioEncoder() { return m_sAudioEncoder; }
    bool         TranscodeVorbis() { return m_bTranscodeVorbis; }
    bool         TranscodeMusePack() { return m_bTranscodeMusePack; }
    bool         TranscodeFlac() { return m_bTranscodeFlac; }*/
  
		
  private:
    std::string				m_sLameLibName;
    std::string				m_sTwoLameLibName;
    std::string				m_sVorbisLibName;
    std::string				m_sMpcLibName;
    std::string				m_sFlacLibName;
    std::string      	m_sFaadLibName;
		std::string				m_sMp4ffLibName;
		std::string				m_sMadLibName;
};

#endif

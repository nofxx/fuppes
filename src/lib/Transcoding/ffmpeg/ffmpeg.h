
#ifndef _FUPPES_FFMPEG_H
#define _FUPPES_FFMPEG_H

#ifdef __cplusplus 
extern "C" 
{ 

  int ffmpeg_main(int argc, char **argv);
  
  void ffmpeg_break();
  
  void ffmpeg_close();
} 
#endif // __cplusplus 


class CFFmpeg {

  public:
    CFFmpeg() {}
    ~CFFmpeg() {}
  
};

#endif // _FUPPES_FFMPEG_H

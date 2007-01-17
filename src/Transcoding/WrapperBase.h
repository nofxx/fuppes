/***************************************************************************
 *            WrapperBase.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 - 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
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
 
#ifndef _WRAPPERBASE_H
#define _WRAPPERBASE_H

#include "../SharedLog.h"
#include <string>

class CAudioEncoderBase
{
};

class CDecoderBase
{
  public:
    virtual bool LoadLib() = 0;
    virtual bool OpenFile(std::string p_sFileName) = 0;      
    virtual void CloseFile() = 0;    
    virtual long DecodeInterleaved(char* p_PcmOut, unsigned int p_nSize) = 0;  
};

class CTranscoderBase
{
  public:
    virtual bool Transcode(std::string p_sInFile, std::string p_sParams, std::string* p_psOutFile) = 0;
};

#endif /* _WRAPPERBASE_H */

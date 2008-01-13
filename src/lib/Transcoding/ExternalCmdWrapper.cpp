/***************************************************************************
 *            ExternalCmdWrapper.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2008 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef DISABLE_TRANSCODING

#include "ExternalCmdWrapper.h"

#include <iostream>
#include <sstream>

#include "../SharedConfig.h"

#ifdef WIN32
#else
#include <stdlib.h>
#endif

using namespace std;

bool CExternalCmdWrapper::Transcode(CFileSettings* pFileSettings, std::string p_sInFile, std::string* p_psOutFile)
{    
  string sTmpFileName = CSharedConfig::Shared()->CreateTempFileName() + "." + pFileSettings->Extension();
	*p_psOutFile = sTmpFileName;
		
	string sCmd = pFileSettings->pTranscodingSettings->ExternalCmd();
	sCmd = StringReplace(sCmd, "%in%", p_sInFile);
	sCmd = StringReplace(sCmd, "%out%", *p_psOutFile);
		
	#ifdef WIN32
  #warning todo: exec
  #else	
  //sDcraw << "dcraw " << pFileSettings->pImageSettings->sDcrawParams << " -T -c " << p_sInFile << " > " << sTmpFileName;
  system(sCmd.c_str());
  #endif

  return true;
}

#endif // DISABLE_TRANSCODING

/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */

#include "../../src/lib/Common/Socket.h"
#include "../../src/lib/Common/Exception.h"

#include <string.h>

#include <sstream>
#include <iostream>
#include <stdio.h>
using namespace std;

void printHeader(fuppes::TCPSocket* socket)
{
  string buffer = socket->buffer();
  size_t pos;
  if((pos = buffer.find("\r\n\r\n")) != string::npos) {
    buffer = buffer.substr(0, pos);
    cout << buffer << endl;    
  }
  else {
    cout << "HEADER NOT FOUND" << endl;  
  }
}

void printMessage(fuppes::TCPSocket* socket)
{
  cout << socket->buffer() << endl;
}

void httpGetFull(std::string address, int port)
{
  stringstream msg;

  msg << "GET /description.xml HTTP/1.0" << "\r\n";
  msg << "Host: " << address << ":" << port << "\r\n";
  //msg << "CONTENT-LENGTH: 0" << "\r\n";
  msg << "User-Agent: TESTOS/OS Version, UPnP/1.0, fuppestest/1.0\r\n";  
  msg << "\r\n";

  
  fuppes::TCPSocket socket("");
 	socket.remoteAddress(address);
	socket.remotePort(port);
  socket.connect();
  socket.send(msg.str());

  try {

    int recv;
    int loop = 0;
    while(loop < 10) {
      recv = socket.receive(1);
      //cout << "received: " << recv << " bytes" << endl;
      loop++;
    }
    
  } catch(fuppes::Exception ex) {
    cout << ex.what() << endl;
  }


  printHeader(&socket);  
}


void httpGetPartial(std::string address, int port)
{
  stringstream msg;

  msg << "GET /MediaServer/VideoItems/0000000021.flv HTTP/1.1" << "\r\n";
  msg << "Host: " << address << ":" << port << "\r\n";
  msg << "Range: bytes=0-" << "\r\n";
  msg << "User-Agent: TESTOS/OS Version, UPnP/1.0, fuppestest/1.0\r\n";  
  msg << "\r\n";
  
  fuppes::TCPSocket socket("");
 	socket.remoteAddress(address);
	socket.remotePort(port);
  socket.connect();
  socket.send(msg.str());

  try {

    int recv;
    int loop = 0;
    while(loop < 10) {
      recv = socket.receive(1);
      //cout << "received: " << recv << " bytes" << endl;
      loop++;
    }
    
  } catch(fuppes::Exception ex) {
    cout << ex.what() << endl;
  }

  printHeader(&socket);
}

void httpHead()
{
}

void httpPostChunked(std::string address, int port)
{
  stringstream msg;
  stringstream tmp;
  
  fuppes::TCPSocket socket("");
 	socket.remoteAddress(address);
	socket.remotePort(port);
  socket.connect();

  char size[10];
  
  msg << "POST /UPnPServices/ContentDirectory/control/ HTTP/1.1\r\n";
  msg << "Host: " << address << ":" << port << "\r\n";
  msg << "Connection: close\r\n";
  msg << "User-Agent: TESTOS/OS Version, UPnP/1.0, fuppestest/1.0\r\n";
  msg << "Accept: */*\r\n";
  msg << "SOAPACTION: \"urn:schemas-upnp-org:service:ContentDirectory:1#Browse\"\r\n";
  msg << "Content-Type: text/xml; charset=utf-8\r\n";
  msg << "Transfer-Encoding: chunked\r\n";
  msg << "\r\n";

  socket.send(msg.str());
  msg.str("");

  usleep(10000);

  tmp << "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n";
  tmp << "<s:Body><u:Browse xmlns:u=\"urn:schemas-upnp-org:service:ContentDirectory:1\">\r\n";
  tmp << " <ObjectID>0</ObjectID>\r\n";
  tmp << " <BrowseFlag>BrowseMetadata</BrowseFlag>\r\n";

  memset(size, 0, sizeof(size));
  sprintf(size, "%X", tmp.str().length());
  msg << size << "\r\n";
  msg << tmp.str();
  tmp.str("");
  socket.send(msg.str());
  msg.str("");
  usleep(10000);
 
  tmp << " <Filter>@id,res,res@resolution,res@size,res@duration,@child_count,upnp:albumArtURI,upnp:genre,upnp:artist,upnp:album,dc:description,dc:title</Filter>";
  tmp << " <StartingIndex>0</StartingIndex>\r\n";
  tmp << " <RequestedCount>1</RequestedCount>\r\n";
  tmp << " <SortCriteria></SortCriteria>\r\n";

  memset(size, 0, sizeof(size));
  sprintf(size, "%X", tmp.str().length());
  msg << size << "\r\n";
  msg << tmp.str();
  tmp.str("");
  socket.send(msg.str());
  msg.str("");
  usleep(10000);
 
  tmp << "</u:Browse>\r\n";
  tmp << "</s:Body>\r\n";
  tmp << "</s:Envelope>\r\n";

  memset(size, 0, sizeof(size));
  sprintf(size, "%X", tmp.str().length());
  msg << size << "\r\n";
  msg << tmp.str();
  tmp.str("");
  socket.send(msg.str());
  msg.str("");


  msg << "\r\n0\r\n";
  socket.send(msg.str());
 
  try {

    int recv;
    int loop = 0;
    while(loop < 10) {
      recv = socket.receive(1);
      //cout << "received: " << recv << " bytes" << endl;
      loop++;
    }
    
  } catch(fuppes::Exception ex) {
    cout << ex.what() << endl;
  }

  printHeader(&socket);
}

void httpPostChunked2(std::string address, int port)
{
  stringstream msg;
  stringstream tmp;
  
  fuppes::TCPSocket socket("");
 	socket.remoteAddress(address);
	socket.remotePort(port);
  socket.connect();

  char size[10];
  
  msg << "POST /UPnPServices/ContentDirectory/control/ HTTP/1.1\r\n";
  msg << "Host: " << address << ":" << port << "\r\n";
  msg << "Connection: close\r\n";
  msg << "User-Agent: TESTOS/OS Version, UPnP/1.0, fuppestest/1.0\r\n";
  msg << "Accept: */*\r\n";
  msg << "SOAPACTION: \"urn:schemas-upnp-org:service:ContentDirectory:1#Browse\"\r\n";
  msg << "Content-Type: text/xml; charset=utf-8\r\n";
  msg << "Transfer-Encoding: chunked\r\n";
  msg << "\r\n";

  tmp << "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n";
  tmp << "<s:Body><u:Browse xmlns:u=\"urn:schemas-upnp-org:service:ContentDirectory:1\">\r\n";
  tmp << " <ObjectID>0</ObjectID>\r\n";
  tmp << " <BrowseFlag>BrowseMetadata</BrowseFlag>\r\n"; 
  tmp << " <Filter>@id,res,res@resolution,res@size,res@duration,@child_count,upnp:albumArtURI,upnp:genre,upnp:artist,upnp:album,dc:description,dc:title</Filter>";
  tmp << " <StartingIndex>0</StartingIndex>\r\n";
  tmp << " <RequestedCount>1</RequestedCount>\r\n";
  tmp << " <SortCriteria></SortCriteria>\r\n"; 
  tmp << "</u:Browse>\r\n";
  tmp << "</s:Body>\r\n";
  tmp << "</s:Envelope>\r\n";

  memset(size, 0, sizeof(size));
  sprintf(size, "00%X", tmp.str().length());
  msg << size << "\r\n";
  msg << tmp.str();
  msg << "\r\n0\r\n";
  tmp.str("");
  socket.send(msg.str());
  msg.str("");

 
  try {

    int recv;
    int loop = 0;
    while(loop < 10) {
      recv = socket.receive(1);
      //cout << "received: " << recv << " bytes" << endl;
      loop++;
    }
    
  } catch(fuppes::Exception ex) {
    cout << ex.what() << endl;
  }

  printHeader(&socket);
}

void httpPostChunkedBrowseDirectChildren(std::string address, int port)
{
  stringstream msg;
  stringstream tmp;
  
  fuppes::TCPSocket socket("");
 	socket.remoteAddress(address);
	socket.remotePort(port);
  socket.connect();

  char size[10];

  
  msg << "POST /UPnPServices/ContentDirectory/control/ HTTP/1.1\r\n";
  msg << "Host: " << address << ":" << port << "\r\n";
  msg << "Connection: close\r\n";
  //msg << "User-Agent: TESTOS/OS Version, UPnP/1.0, fuppestest/1.0\r\n";
  msg << "User-Agent:  Moxi/6.0 MoxiHDID/00067F258BBE Allegro-Software-WebClient/5.10b1 DLNADOC/1.50\r\n";
  msg << "Accept: */*\r\n";
  msg << "SOAPACTION: \"urn:schemas-upnp-org:service:ContentDirectory:1#Browse\"\r\n";
  msg << "Content-Type: text/xml; charset=utf-8\r\n";
  msg << "Transfer-Encoding: chunked\r\n";
  msg << "\r\n";

  tmp << "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n";
  tmp << " <s:Body><u:Browse xmlns:u=\"urn:schemas-upnp-org:service:ContentDirectory:1\">\r\n";
  tmp << " <ObjectID>0</ObjectID>\r\n";
  tmp << " <BrowseFlag>BrowseDirectChildren</BrowseFlag>\r\n";
  tmp << " <Filter>*</Filter>\r\n";
  tmp << " <StartingIndex>0</StartingIndex>\r\n";
  tmp << " <RequestedCount>20</RequestedCount>\r\n";
  tmp << " <SortCriteria></SortCriteria>\r\n";
  tmp << "</u:Browse>\r\n";
  tmp << "</s:Body>\r\n";
  tmp << "</s:Envelope>\r\n";
  
  memset(size, 0, sizeof(size));
  sprintf(size, "00%X", tmp.str().length());

 
  msg << size << "\r\n";      // size crlf
  msg << tmp.str() << "\r\n"; // chunk crlf
  msg << "0\r\n\r\n";             // end crlf
  tmp.str("");

cout << msg.str() << endl;
 
  socket.send(msg.str());
  msg.str("");

 
  try {

    int recv;
    int loop = 0;
    while(loop < 10) {
      recv = socket.receive(1);
      //cout << "received: " << recv << " bytes" << endl;
      loop++;
    }
    
  } catch(fuppes::Exception ex) {
    cout << ex.what() << endl;
  }

  printMessage(&socket);
}



void httpPostChunkedBrowseMetadata(std::string address, int port)
{
  stringstream msg;
  stringstream tmp;
  
  fuppes::TCPSocket socket("");
 	socket.remoteAddress(address);
	socket.remotePort(port);
  socket.connect();

  char size[10];

  
  msg << "POST /UPnPServices/ContentDirectory/control/ HTTP/1.1\r\n";
  msg << "Host: " << address << ":" << port << "\r\n";
  msg << "Connection: close\r\n";
  //msg << "User-Agent: TESTOS/OS Version, UPnP/1.0, fuppestest/1.0\r\n";
  msg << "User-Agent:  Moxi/6.0 MoxiHDID/00067F258BBE Allegro-Software-WebClient/5.10b1 DLNADOC/1.50\r\n";
  msg << "Accept: */*\r\n";
  msg << "SOAPACTION: \"urn:schemas-upnp-org:service:ContentDirectory:1#Browse\"\r\n";
  msg << "Content-Type: text/xml; charset=utf-8\r\n";
  msg << "Transfer-Encoding: chunked\r\n";
  msg << "\r\n";

  tmp << "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n";
  tmp << " <s:Body><u:Browse xmlns:u=\"urn:schemas-upnp-org:service:ContentDirectory:1\">\r\n";
  tmp << " <ObjectID>0000001387</ObjectID>\r\n";
  tmp << " <BrowseFlag>BrowseMetadata</BrowseFlag>\r\n";
  tmp << " <Filter>@id,res,res@resolution,res@size,res@duration,@child_count,upnp:albumArtURI,upnp:genre,upnp:artist,upnp:album,dc:description,dc:title</Filter>\r\n";
  tmp << " <StartingIndex>0</StartingIndex>\r\n";
  tmp << " <RequestedCount>1</RequestedCount>\r\n";
  tmp << " <SortCriteria></SortCriteria>\r\n";
  tmp << "</u:Browse>\r\n";
  tmp << "</s:Body>\r\n";
  tmp << "</s:Envelope>\r\n";
  
  memset(size, 0, sizeof(size));
  sprintf(size, "00%X", tmp.str().length());

 
  msg << size << "\r\n";      // size crlf
  msg << tmp.str() << "\r\n"; // chunk crlf
  msg << "0\r\n\r\n";             // end crlf
  tmp.str("");

cout << msg.str() << endl;
 
  socket.send(msg.str());
  msg.str("");

 
  try {

    int recv;
    int loop = 0;
    while(loop < 10) {
      recv = socket.receive(1);
      //cout << "received: " << recv << " bytes" << endl;
      loop++;
    }
    
  } catch(fuppes::Exception ex) {
    cout << ex.what() << endl;
  }

  printMessage(&socket);
}



int main(int argc, char* argv[])
{
  
  // dlna 1.0 clients may not send the transferMode header.
  // in that case we have to treat it as "Streaming" for Audio and Video objects
  // and "Interactive" for all other binaries



  /*cout << "httpGetFull()" << endl;
  httpGetFull("192.168.0.8", 5080);
  getchar();
  
  cout << "httpGetPartial()" << endl;
  httpGetPartial("192.168.0.8", 5080);  
  getchar();*/

  cout << "httpPostChunkedBrowseDirectChildren()" << endl;
  httpPostChunkedBrowseDirectChildren("192.168.0.8", 5080);  
  getchar();


  cout << "httpPostChunkedBrowseMetadata()" << endl;
  httpPostChunkedBrowseMetadata("192.168.0.8", 5080);  
  getchar();
  

  
  return 0;
}

// ./configure --prefix=/usr --enable-lame --enable-transcoder-ffmpeg --enable-vfolder=no --enable-tests=yes

/***************************************************************************
                          knmimeinfo.cpp  -  description
                             -------------------
   
    copyright            : (C) 2000 by Christian Thurner
    email                : cthurner@freepage.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>

#include "knmimeinfo.h"
#include "knmimecontent.h"


KNMimeInfo::KNMimeInfo()
{
  clear();
}



KNMimeInfo::~KNMimeInfo()
{
}



void KNMimeInfo::operator=(const KNMimeInfo &i)
{
  c_ontentType=i.c_ontentType.copy();
  c_tCategory=i.c_tCategory;
  c_tMType=i.c_tMType;
  c_tSType=i.c_tSType;
  c_tEncoding=i.c_tEncoding;
  c_tDisposition=i.c_tDisposition;
  d_ecoded=i.d_ecoded;
}



void KNMimeInfo::clear()
{
  c_tMType=MTapplication;
  c_tSType=SToctetStream;
  c_tEncoding=ECsevenBit;
  c_tDisposition=DPinline;
  c_tCategory=CCsingle;
  d_ecoded=false;
  c_ontentType="";
}



bool KNMimeInfo::isReadable()
{
  return (c_tMType==MTtext && ( c_tEncoding==ECsevenBit || c_tEncoding==ECeightBit || d_ecoded) );
}



bool KNMimeInfo::decoded()
{
  return ( c_tEncoding==ECsevenBit || c_tEncoding==ECeightBit || d_ecoded );
}
    


void KNMimeInfo::parse(KNMimeContent *c)
{
  QCString tmp;
  tmp=c->headerLine("Content-Type");
  c_ontentType=tmp;
  if(tmp.isEmpty()) {
    if(c->type()==ATmimeContent) {
      c_tMType=MTapplication;
      c_tSType=SToctetStream;
    }
    else {
      c_tMType=MTtext;
      c_tSType=STplain;
    }
  }
  else
    parseMimeType(tmp);   
    
  tmp=c->headerLine("Content-Transfer-Encoding");
  parseEncoding(tmp);
  
  tmp=c->headerLine("Content-Disposition");
  parseDisposition(tmp);
}



void KNMimeInfo::parseMimeType(const QCString &s)
{
  if(s.find("text", 0, false)!=-1)              c_tMType=MTtext;
  else if(s.find("image", 0, false)!=-1)        c_tMType=MTimage;
  else if(s.find("audio", 0, false)!=-1)        c_tMType=MTaudio;
  else if(s.find("video", 0, false)!=-1)        c_tMType=MTvideo;
  else if(s.find("multipart", 0, false)!=-1)    c_tMType=MTmultipart;
  else if(s.find("message", 0, false)!=-1)      c_tMType=MTmessage;
  else if(s.find("application", 0, false)!=-1)  c_tMType=MTapplication;
  else                                          c_tMType=MTcustom;                                            
  
  switch(c_tMType) {
  
    case MTtext:
      if(s.find("/plain", 0, false)!=-1)              c_tSType=STplain;
      else if(s.find("/html", 0, false)!=-1)          c_tSType=SThtml;
      else if(s.find("/enriched", 0, false)!=-1)      c_tSType=STenriched;
      else                                            c_tSType=STcustom;
    break;
  
    case MTimage:
      if(s.find("/gif", 0, false)!=-1)                c_tSType=STgif;
      else if(s.find("/jpeg", 0, false)!=-1)          c_tSType=STjpeg;
      else                                            c_tSType=STcustom;
    break;
  
    case MTaudio:
      if(s.find("/basic", 0, false)!=-1)              c_tSType=STbasic;
      else                                            c_tSType=STcustom;
    break;
  
    case MTvideo:
      if(s.find("/mpeg", 0, false)!=-1)               c_tSType=STmpeg;
      else                                            c_tSType=STcustom;
    break;
      
    case MTapplication:
      if(s.find("/postscript", 0, false)!=-1)         c_tSType=STPostScript;
      else if(s.find("/octet-stream", 0, false)!=-1)  c_tSType=SToctetStream;
      else                                            c_tSType=STcustom;
    break;
  
    case MTmultipart:
      if(s.find("/alternative", 0, false)!=-1)        c_tSType=STalternative;
      else if(s.find("/parallel", 0, false)!=-1)      c_tSType=STparallel;
      else if(s.find("/digest", 0, false)!=-1)        c_tSType=STdigest;
      else                                            c_tSType=STmixed;
    break;
  
    case MTmessage:
      if(s.find("/partial", 0, false)!=-1)            c_tSType=STpartial;
      else if(s.find("/external-body", 0, false)!=-1) c_tSType=STexternalBody;
      else                                            c_tSType=STrfc822;
    break;
    default:                                          c_tSType=STcustom;
  }
}



void KNMimeInfo::parseEncoding(const QCString &s)
{
  if(s.isEmpty()) {
    if(c_tMType==MTtext)                          c_tEncoding=ECsevenBit;
    else                                          c_tEncoding=ECbinary;
  }
  else
    c_tEncoding=stringToEncoding(s.data());
  
  /*if(strcasecmp(s,"7bit")==0)               c_tEncoding=ECsevenBit;
  else if(strcasecmp(s,"8bit")==0)                c_tEncoding=ECeightBit;
  else if(strcasecmp(s,"quoted-printable")==0)    c_tEncoding=ECquotedPrintable;
  else if(strcasecmp(s,"base64")==0)              c_tEncoding=ECbase64;
  else if(strcasecmp(s,"x-uuencode")==0)          c_tEncoding=ECuuencode;
  else                                            c_tEncoding=ECbinary;*/
        
}



void KNMimeInfo::parseDisposition(const QCString &s)
{
  if(s.isEmpty())                             c_tDisposition=DPinline;
  else if(strcasecmp(s.data(),"inline")==0)   c_tDisposition=DPinline;
  else                                        c_tDisposition=DPattached;
}



const QCString& KNMimeInfo::contentType()
{
  QCString params;
  int pos1;

  pos1=c_ontentType.find('/');
  if(c_tMType==MTcustom || c_tSType==STcustom) {
    if(pos1==-1)
      c_ontentType="application/octet-stream";
    return c_ontentType;
  }

  if(pos1==-1) c_ontentType.prepend(assembleMimeType());
  else {
    pos1=c_ontentType.find(';', pos1);
    if(pos1==-1) c_ontentType=assembleMimeType();
    else {
      params=c_ontentType.right(c_ontentType.length()-pos1);
      c_ontentType=assembleMimeType()+params;
    }
  }
  return c_ontentType;
}



QCString KNMimeInfo::contentTransferEncoding()
{
  QCString ret=encodingToString(c_tEncoding);

  if(ret.isEmpty() && c_tMType==MTtext)
    ret="7Bit";

  return ret;
}



QCString KNMimeInfo::contentDisposition()
{
  QCString disp;
  if(c_tDisposition==DPattached) disp="attachment";
  else disp="inline";

  return disp;
}



QCString KNMimeInfo::getCTParameter(const char* param)
{
  QCString ret;
  int pos1=0, pos2=0;
  if(!c_ontentType.isEmpty()) {
    pos1=c_ontentType.find(param, 0, false);
    if(pos1!=-1) {
      pos2=c_ontentType.find(';', pos1);
      if (pos2==-1) pos2=c_ontentType.length();
      pos1+=strlen(param)+1;
      ret=c_ontentType.mid(pos1, pos2-pos1);
      removeQuots(ret);
    }
  }
  kdDebug(5003) << "KNMimeInfo::getCTParameter() : " << param << " = " << ret << endl;
  return ret;
}



bool KNMimeInfo::ctParameterIsSet(const char *param)
{
  return ( c_ontentType.find(param, 0, false)!=-1 );
}




void KNMimeInfo::setCTParameter(const QCString &name, const QCString &value, bool doubleQuotes)
{
  int pos1=0, pos2=0;

  QCString param;

  if(doubleQuotes)
    param=name+"=\""+value+"\"";
  else
    param=name+"="+value;

  pos1=c_ontentType.find(name, 0, false);
  if(pos1==-1) {
    c_ontentType+="; "+param;
  }
  else {
    pos2=c_ontentType.find(';', pos1);
    if(pos2==-1)
      pos2=c_ontentType.length();
    c_ontentType.remove(pos1, pos2-pos1);
    c_ontentType.insert(pos1, param);
  }
}



void KNMimeInfo::setCharsetParameter(const QCString &p)
{
  setCTParameter("charset", p, false);
}



void KNMimeInfo::setBoundaryParameter(const QCString &p)
{
  setCTParameter("boundary", p);
}



void KNMimeInfo::setNameParameter(const QCString &p)
{
  setCTParameter("name", p);
}



void KNMimeInfo::setCustomMimeType(const QCString &m)
{
  int pos=c_ontentType.find(';');

  if(pos==-1)
    c_ontentType=m;
  else {
    QCString params=c_ontentType.right(c_ontentType.length()-pos);
    c_ontentType=m+params;
  }

  parseMimeType(c_ontentType);

}



/*void KNMimeInfo::addCTParameter(const QCString &s)
{
  c_ontentType+="; "+s;
}*/


QCString KNMimeInfo::assembleMimeType()
{
  QCString m;

  switch(c_tMType) {

    case MTtext:      m="text/";        break;
    case MTimage:     m="image/";       break;
    case MTaudio:     m="audio/";       break;
    case MTvideo:     m="video/";       break;
    case MTmultipart: m="multipart/";   break;
    case MTmessage:   m="message/";     break;
    default:          m="application/"; break;
  }

  switch(c_tSType)  {
    case STplain:          m+="plain";         break;
    case SThtml:           m+="html";          break;
    case STenriched:       m+="enriched";      break;
    case STgif:            m+="gif";           break;
    case STjpeg:           m+="jpeg";          break;
    case STbasic:          m+="basic";         break;
    case STmpeg:           m+="mpeg";          break;
    case STPostScript:     m+="PostScript";    break;
    case STmixed:          m+="mixed";         break;
    case STalternative:    m+="alternative";   break;
    case STparallel:       m+="parallel";      break;
    case STdigest:         m+="digest";        break;
    case STrfc822:         m+="rfc822";        break;
    case STpartial:        m+="partial";       break;
    case STexternalBody:   m+="externalBody";  break;
    default:               m+="octet-stream";  break;
  }

  return m;
}



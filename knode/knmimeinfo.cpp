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


#include "knmimeinfo.h"
#include "knmimecontent.h"

KNMimeInfo::KNMimeInfo()
{
	c_tMType=MTapplication;
	c_tSType=SToctetStream;
	c_tEncoding=ECsevenBit;
	c_tDisposition=DPinline;
	c_tCategory=CCmain;
	i_sReadable=true;	
}



KNMimeInfo::~KNMimeInfo()
{
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
  if(s.find("text", 0, false)!=-1) 							c_tMType=MTtext;
 	else if(s.find("image", 0, false)!=-1) 				c_tMType=MTimage;
 	else if(s.find("audio", 0, false)!=-1)      	c_tMType=MTaudio;
 	else if(s.find("video", 0, false)!=-1)      	c_tMType=MTvideo;
 	else if(s.find("multipart", 0, false)!=-1)		c_tMType=MTmultipart;
 	else if(s.find("message", 0, false)!=-1)    	c_tMType=MTmessage;
 	else 																					c_tMType=MTapplication;																						
	
 	switch(c_tMType) {
	
 		case MTtext:
 			if(s.find("/plain", 0, false)!=-1)							c_tSType=STplain;
 			else if(s.find("/html", 0, false)!=-1)					c_tSType=SThtml;
 			else if(s.find("/enriched", 0, false)!=-1)			c_tSType=STenriched;
 		break;
	
 		case MTimage:
 			if(s.find("/gif", 0, false)!=-1)								c_tSType=STgif;
 			else if(s.find("/jpeg", 0, false)!=-1)					c_tSType=STjpeg;
 		break;
	
 		case MTaudio:
 			if(s.find("/basic", 0, false)!=-1)							c_tSType=STbasic;
 		break;
	
 		case MTvideo:
 			if(s.find("/mpeg", 0, false)!=-1)							  c_tSType=STmpeg;
 		break;
			
 		case MTapplication:
 			if(s.find("/postscript", 0, false)!=-1)				  c_tSType=STPostScript;
 			else if(s.find("/octet-stream", 0, false)!=-1)	c_tSType=SToctetStream;
 		break;
	
 		case MTmultipart:
 			if(s.find("/alternative", 0, false)!=-1)				c_tSType=STalternative;
 			else if(s.find("/parallel", 0, false)!=-1)			c_tSType=STparallel;
 			else if(s.find("/digest", 0, false)!=-1)  			c_tSType=STdigest;
 			else																					  c_tSType=STmixed;
 		break;
	
 		case MTmessage:
 			if(s.find("/rfc822", 0, false)!=-1)						  c_tSType=STrfc822;
 			else if(s.find("/partial", 0, false)!=-1)				c_tSType=STpartial;
 			else if(s.find("/external-body", 0, false)!=-1)	c_tSType=STexternalBody;
 		break;
		default: 																				  c_tSType=SToctetStream;
 	}
}



void KNMimeInfo::parseEncoding(const QCString &s)
{
  if(s.isEmpty()) {
    if(c_tMType==MTtext)							            c_tEncoding=ECsevenBit;
    else                                          c_tEncoding=ECbinary;
  }
	else if(strcasecmp(s,"7bit")==0)          			c_tEncoding=ECsevenBit;
	else if(strcasecmp(s,"8bit")==0)          			c_tEncoding=ECeightBit;
	else if(strcasecmp(s,"quoted-printable")==0)   	c_tEncoding=ECquotedPrintable;
	else if(strcasecmp(s,"base64")==0)          		c_tEncoding=ECbase64;
	else if(strcasecmp(s,"x-uuencode")==0)          c_tEncoding=ECuuencode;
	else           																	c_tEncoding=ECbinary;
		
	i_sReadable=(c_tEncoding==ECsevenBit || c_tEncoding==ECeightBit);
			
}



void KNMimeInfo::parseDisposition(const QCString &s)
{
  if(s.isEmpty())											        c_tDisposition=DPinline;
	else if(strcasecmp(s.data(),"inline")==0)   c_tDisposition=DPinline;
  else                                        c_tDisposition=DPattached;
}



const QCString& KNMimeInfo::contentType()
{
  QCString params;
  int pos1;

  pos1=c_ontentType.find('/');
  if(c_tMType==MTcustom && pos1==-1) {
    c_ontentType="application/octet-stream";
    return c_ontentType;
  }

  if(pos1==-1) c_ontentType.prepend(assembleMimeType());
  else {
    pos1=c_ontentType.find(';', pos1);
    if(pos1==-1) c_ontentType=assembleMimeType();
    else {
      params=c_ontentType.right(c_ontentType.length()-pos1+1);
      c_ontentType=assembleMimeType()+params;
    }
  }
  return c_ontentType;
}



QCString KNMimeInfo::contentTransferEncoding()
{
  QCString ret;

  switch(c_tEncoding) {
    case ECsevenBit:        ret="7Bit";              break;
    case ECeightBit:        ret="8Bit";              break;
    case ECquotedPrintable: ret="quoted-printable";  break;
    case ECbase64:          ret="base64";            break;
    case ECuuencode:        ret="x-uuencode";        break;
    default:
      if(c_tMType==MTtext)
        ret="7Bit";
      else
        ret="binary";
  }
  return ret;
}



QCString KNMimeInfo::contentDisposition()
{
  QCString disp;
  if(c_tDisposition==DPattached) disp="attached";
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
	qDebug("KNMimeInfo::getCTParameter() : %s = %s", param, ret.data());
	return ret;
}



void KNMimeInfo::setCustomMimeType(const QCString &m)
{
  c_ontentType=m.copy();

  parseMimeType(c_ontentType);
  if(c_tMType==MTapplication && c_tSType==SToctetStream) {
    c_tMType=MTcustom;
    c_tSType=STcustom;
  }
}



void KNMimeInfo::addCTParameter(const QCString &s)
{
  c_ontentType+="; "+s;
}



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



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
	c_tMType=MTtext;
	c_tSType=STplain;
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
  int pos1=0;
  tmp=c->headerLine("Content-Type");
		
	if(!tmp.isEmpty()) {
		c_ontentType=tmp;
		pos1=tmp.find(';');
		if(pos1!=-1) {
			tmp=tmp.left(pos1);
		}
				
		if(tmp.find("text", 0, false)!=-1) 							c_tMType=MTtext;
		else if(tmp.find("image", 0, false)!=-1) 				c_tMType=MTimage;
		else if(tmp.find("audio", 0, false)!=-1)      	c_tMType=MTaudio;
		else if(tmp.find("video", 0, false)!=-1)      	c_tMType=MTvideo;
		else if(tmp.find("multipart", 0, false)!=-1)		c_tMType=MTmultipart;
		else if(tmp.find("message", 0, false)!=-1)    	c_tMType=MTmessage;
		else 																						c_tMType=MTapplication;																						
	
		switch(c_tMType) {
	
			case MTtext:
				if(tmp.find("/plain", 0, false)!=-1)							c_tSType=STplain;
				else if(tmp.find("/html", 0, false)!=-1)					c_tSType=SThtml;
				else if(tmp.find("/enriched", 0, false)!=-1)			c_tSType=STenriched;
			break;
	
			case MTimage:
				if(tmp.find("/gif", 0, false)!=-1)								c_tSType=STgif;
				else if(tmp.find("/jpeg", 0, false)!=-1)					c_tSType=STjpeg;
			break;
	
			case MTaudio:
				if(tmp.find("/basic", 0, false)!=-1)							c_tSType=STbasic;
			break;
	
			case MTvideo:
				if(tmp.find("/mpeg", 0, false)!=-1)							  c_tSType=STmpeg;
			break;
			
			case MTapplication:
				if(tmp.find("/postscript", 0, false)!=-1)				  c_tSType=STPostScript;
				else if(tmp.find("/octet-stream", 0, false)!=-1)	c_tSType=SToctetStream;
			break;
	
			case MTmultipart:
				if(tmp.find("/alternative", 0, false)!=-1)				c_tSType=STalternative;
				else if(tmp.find("/parallel", 0, false)!=-1)			c_tSType=STparallel;
				else if(tmp.find("/digest", 0, false)!=-1)  			c_tSType=STdigest;
				else																						  c_tSType=STmixed;
			break;
	
			case MTmessage:
				if(tmp.find("/rfc822", 0, false)!=-1)						  c_tSType=STrfc822;
				else if(tmp.find("/partial", 0, false)!=-1)				c_tSType=STpartial;
				else if(tmp.find("/external-body", 0, false)!=-1)	c_tSType=STexternalBody;
			break;
	
			default: 																				    c_tSType=SToctetStream;
		}
	}
	
	tmp=c->headerLine("Content-Transfer-Encoding");
	if(tmp.isEmpty()) 																	c_tEncoding=ECsevenBit;
	else if(strcasecmp(tmp,"7bit")==0)          				c_tEncoding=ECsevenBit;
	else if(strcasecmp(tmp,"8bit")==0)          				c_tEncoding=ECeightBit;
	else if(strcasecmp(tmp,"quoted-printable")==0)   	 	c_tEncoding=ECquotedPrintable;
	else if(strcasecmp(tmp,"base64")==0)          			c_tEncoding=ECbase64;
	else if(strcasecmp(tmp,"x-uuencode")==0)          	c_tEncoding=ECuuencode;
	else           																			c_tEncoding=ECbinary;
		
	i_sReadable=(c_tEncoding==ECsevenBit || c_tEncoding==ECeightBit);
			
	tmp=c->headerLine("Content-Disposition");
	if(tmp.isEmpty())																		c_tDisposition=DPinline;
	else if(strcasecmp(tmp,"inline")==0)          			c_tDisposition=DPinline;
  else                                                c_tDisposition=DPattached;

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
  c_tMType=MTcustom;
  c_tSType=STcustom;
  c_ontentType=m.copy();
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



/***************************************************************************
                          knmimecontent.cpp  -  description
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

#include <qtextstream.h>

#include <mimelib/string.h>
#include <mimelib/utility.h>
#include <mimelib/uuencode.h>

#include <klocale.h>

#include "knarticlecollection.h"
#include "knstringsplitter.h"
#include "knmimecontent.h"


KNMimeContent::KNMimeContent()
{
	h_ead=0;
	b_ody=0;
	ct_List=0;
	mInfo=0;
}



KNMimeContent::~KNMimeContent()
{
	delete mInfo;
	delete b_ody;
	delete h_ead;
	delete ct_List;	
}



void KNMimeContent::initContent()
{
	if(h_ead) h_ead->clear();
	else {
		h_ead=new QStrList();
		h_ead->setAutoDelete(true);	
	}
		
	if(b_ody) b_ody->clear();
	else {
  	b_ody=new QStrList();
		b_ody->setAutoDelete(true);
	}
	if(!mInfo) mInfo=new KNMimeInfo();
}



void KNMimeContent::setData(QStrList *data, bool crlf)
{
	bool isHead=true;
	char *line=data->first();
	
	initContent();
	
	while(isHead && line) {
		if(crlf) isHead=(strcmp(line, "\r\n")!=0);
		else isHead=(strcmp(line, "")!=0);
		if(isHead) {
			if(crlf) stripCRLF(line);
			h_ead->append(line);
			line=data->next();
		}		
	}
	
	line=data->next();
	
	while(line) {
		if(crlf) stripCRLF(line);
		b_ody->append(line);
		line=data->next();
	}
}



void KNMimeContent::clear()
{
	if(b_ody) b_ody->clear();
	if(h_ead) h_ead->clear();
	if(ct_List) ct_List->clear();
	if(mInfo) {
		delete mInfo;
		mInfo=0;
	}	
}



void KNMimeContent::parse()
{
	QCString tmp;
	KNMimeContent *mCT=0, *uuContent=0;
	QStrList *part;
	bool mainContentFound=false;
	contentCategory cat=CCmain;
		
	//parse Header
	mimeInfo()->parse(this);

  //parse Body
  if(b_ody && mInfo->ctMediaType()==MTmultipart) {
  	mInfo->setCTCategory(CCcontainer);
  	if(mInfo->ctSubType()==STalternative) cat=CCalternative;
  	else cat=CCattachement;
  	
  	tmp=mInfo->getCTParameter("boundary");
  	if(!tmp.isEmpty()) {
  		if(!ct_List) {
  			ct_List=new QList<KNMimeContent>;
  			ct_List->setAutoDelete(true);
  		}
  		else ct_List->clear();
  		  		
  		MultiPartParser mpp(b_ody, tmp);
  		
  		part=mpp.begin();
  		
  		while(part) {
  			mCT=new KNMimeContent();
  			mCT->setData(part, false);
  			mCT->parse();
  			if( this->type()!=ATmimeContent &&
  			    !mainContentFound &&
  			    mCT->mimeInfo()->ctMediaType()==MTtext &&
  			    ( mCT->mimeInfo()->ctSubType()==STplain ||
  			      mCT->mimeInfo()->ctSubType()==SThtml ) &&
  			    mCT->mimeInfo()->ctDisposition()==DPinline ) {
  			      mCT->mimeInfo()->setCTCategory(CCmain);
  			      mainContentFound=true;
  			}
        else  mCT->mimeInfo()->setCTCategory(cat);
  			
  			ct_List->append(mCT);
  		  delete part;
  		  part=mpp.nextPart();
  		};
  	
  		if(ct_List->isEmpty()) {
  			delete ct_List;
  			ct_List=0;
  		}	
  		else {
  			delete b_ody;
  			b_ody=0;
  		}
  	}
  }
  else if(b_ody && mInfo->ctMediaType()==MTtext && b_ody->count()>100) {
 		qDebug("KNMimeContent::parse() : uuencoded binary assumed");
 		UUParser uup(b_ody);
 		uup.parse();
 		if(uup.isUUencoded()) {
 			qDebug("KNMimeContent::parse() : is uuencoded");
 			if(!ct_List) {
  			ct_List=new QList<KNMimeContent>;
  			ct_List->setAutoDelete(true);
  		}
  		else ct_List->clear();
 			mInfo->setCTCategory(CCcontainer);
 			mInfo->setCTMediaType(MTmultipart);
 			mInfo->setCTSubType(STmixed);
 			this->KNMimeContent::assemble();
 			uuContent=new KNMimeContent();
 			uuContent->initContent();
 			uuContent->mimeInfo()->setCTMediaType(MTtext);
 			uuContent->mimeInfo()->setCTSubType(STplain);
 			uuContent->mimeInfo()->setCTEncoding(ECsevenBit);
 			uuContent->mimeInfo()->setCTDisposition(DPattached);
 			uuContent->mimeInfo()->setCTCategory(CCmain);
 			uuContent->assemble();
 			part=uup.textPart();
 			for(char *l=part->first(); l; l=part->next())
 				uuContent->addBodyLine(l);
 			ct_List->append(uuContent);
  			
 			uuContent=new KNMimeContent();
 			uuContent->initContent();
 			tmp=uup.assumedMimeType().copy()+"; name=\""+uup.fileName()+"\"";
 			uuContent->mimeInfo()->setCustomMimeType(tmp);
 			uuContent->mimeInfo()->setCTEncoding(ECuuencode);
 			uuContent->mimeInfo()->setCTDisposition(DPattached);
 			uuContent->mimeInfo()->setCTCategory(CCattachement);
 			uuContent->assemble();
 			part=uup.binaryPart();
 			for(char *l=part->first(); l; l=part->next())
 				uuContent->addBodyLine(l);
 			ct_List->append(uuContent);
  		delete b_ody;
 			b_ody=0;
 		}
 	}
}



void KNMimeContent::assemble()
{
  QCString tmp;
  KNMimeInfo *i=mimeInfo();

  if(this->type()!=ATmimeContent) {
    tmp="1.0";
    setHeader(HTmimeVersion, tmp, false);
  }
  else {
    tmp=i->contentDisposition();
    if(!tmp.isEmpty())
      setHeader(HTdisposition, tmp, false);
  }

  tmp=i->contentTransferEncoding();
  if(!tmp.isEmpty())
    setHeader(HTencoding, tmp, false);

  tmp=i->contentType();
  if(!tmp.isEmpty())
    setHeader(HTcontentType, tmp, false);
}



KNMimeInfo* KNMimeContent::mimeInfo()
{
  if(!mInfo) mInfo=new KNMimeInfo();
  return mInfo;
}



void KNMimeContent::prepareForDisplay()
{
	
	if(mInfo->isReadable()) return;
		
	char *line;
	QCString tmp;
	KNStringSplitter split;
		
	if(mInfo->ctMediaType()==MTtext) {
		tmp.resize(16000);
		tmp[0]='\0';
		for(line=firstBodyLine(); line; line=nextBodyLine()){
			tmp+=line;
			tmp+="\n";
		}
		if(mInfo->ctEncoding()==ECquotedPrintable)
			tmp=decodeQuotedPrintable(tmp);
		else if(mInfo->ctEncoding()==ECbase64)
		  tmp=decodeBase64(tmp);	
		b_ody->clear();
		
		split.init(tmp, "\n");
		
		
		for(bool i=split.first(); i; i=split.next()) {
			b_ody->append(split.string());
		}
		
		mInfo->setIsReadable(true);
	}
}



void KNMimeContent::prepareHtml()
{
	if(!b_ody) return;
	QCString tmp;
	int pos=0;
		
	for(char *line=b_ody->first(); line; line=b_ody->next()) {
		tmp=line;
		pos=tmp.find("<html>", 0, false);
		if(pos!=-1) {
			tmp.remove(pos, pos+6);
		  pos=b_ody->at();
		  b_ody->remove();
		  if(!tmp.isEmpty()) b_ody->insert(pos, tmp);
			for(int i=0; i<pos; i++)
				b_ody->remove(b_ody->at(i));
			break;
		}
	}
	for(char *line=b_ody->next(); line; line=b_ody->next()) {	
		tmp=line;
		pos=tmp.find("</html>", 0, false);
		if(pos!=-1) {
			tmp.remove(pos, tmp.length()-pos);
			pos=b_ody->at();
			b_ody->remove();
			if(!tmp.isEmpty()) b_ody->insert(pos, tmp);
			line=b_ody->next();
			if(line)
			  while(b_ody->remove());
			break;
		}
	}
}



KNMimeContent* KNMimeContent::mainContent()
{
	KNMimeContent *ret=0;
	if(this->isMainContent()) ret=this;
	else if(ct_List) {
		for(KNMimeContent *var=ct_List->first(); var; var=ct_List->next()) {
			ret=var->mainContent();
			if(ret) break;
		}
	}
	return ret;
}


void KNMimeContent::attachements(QList<KNMimeContent> *dst, bool incAlternatives)
{
	if(!ct_List) {
		dst->append(this);
	}
	else {
		for(KNMimeContent *var=ct_List->first(); var; var=ct_List->next()) {
		  if(var->isMainContent() || (!incAlternatives && var->mimeInfo()->ctCategory()==CCalternative) )
		    continue;
		  else
			  var->attachements(dst, incAlternatives);
	  }
	}
}



QCString KNMimeContent::ctCharset()
{
	QCString ret;
	ret=mimeInfo()->getCTParameter("charset").upper();
	if(ret.isEmpty()) ret="US-ASCII";
	return ret;
}



QCString KNMimeContent::ctMimeType()
{
	QCString ret, tmp;
	int pos1=0;
	
	tmp=headerLine("Content-Type");
	if(tmp.isEmpty()) ret="application/octet-stream";
	else {
		pos1=tmp.find(';');
		if(pos1==-1) ret=tmp;
		else ret=tmp.left(pos1);
	}
	return ret;	
}



QCString KNMimeContent::ctName()
{
	QCString ret, tmp;
	int pos1=0 , pos2=0;
	
	ret=mimeInfo()->getCTParameter("name");
	if(ret.isEmpty()) {
		tmp=headerLine("Content-Disposition");
		if(tmp.isEmpty()) ret=i18n("unknown").local8Bit();
		else {
			pos1=tmp.find("filename=");
			ret="";
			if(pos1!=-1) {
				pos2=tmp.find(';', pos1);
				if(pos2==-1) pos2=tmp.length();
				pos1+=9;
				ret=tmp.mid(pos1, pos2-pos1);
			}
			if(ret.isEmpty()) ret=i18n("unknown").local8Bit();
		}	
	}
	return ret;
}



QCString KNMimeContent::ctDescription()
{
	QCString ret;
	ret=headerLine("Content-Description");
	if(ret.isEmpty()) ret=i18n("none").local8Bit();
	return ret;
}



void KNMimeContent::addHeaderLine(const char *line, bool encode)
{
	if(h_ead) {
		if(encode) h_ead->append(encodeRFC1522String(line));
		else h_ead->append(line);
	}
}



void KNMimeContent::setHeader(const char *name, const QCString &value, bool encode)
{
	int at;
	QCString line(128);
	line="";
	line+=name;
	line+=": ";
	if(!allow8bit && encode) line+=encodeRFC1522String(value);
	else line+=value;
		
	if(removeHeader(name)) {
		at=h_ead->at();
		if(at==-1) h_ead->append(line);
		else h_ead->insert(at, line);
	}
	else h_ead->append(line);
}



void KNMimeContent::setHeader(headerType t, const QCString &value, bool encode)
{
	char *line;
	int insPos=-1;
	int ht;
	QCString hdr;
	
	for(line=h_ead->first(); line; line=h_ead->next()) {
		ht=stringToHeaderType(line);
		if(ht==t) {
			insPos=h_ead->at();
			h_ead->remove(insPos);
			break;
		}
		if(ht>t) {
			insPos=h_ead->at();
			break;
		}
		else insPos=h_ead->at()+1;
	}
	
	hdr=headerTypeToString(t)+": ";
	if(!allow8bit && encode) hdr+=encodeRFC1522String(value);
	else hdr+=value;
	if(insPos==-1) h_ead->append(hdr);
	else h_ead->insert(insPos, hdr);		
}



bool KNMimeContent::removeHeader(const char* name)
{
	int len=strlen(name);
	
	if(!h_ead) return false;
	for(char *var=h_ead->first(); var; var=h_ead->next())
		if(strncasecmp(var, name, len)==0)
			return h_ead->removeRef(var);
			
  return false;
		
	
}
			


QCString KNMimeContent::headerLine(const char* name)
{
	QCString ret;
	int len=strlen(name);//, pos=0;
	char *var, *line;
	
	for(var=h_ead->first(); var; var=h_ead->next())
		if(strncasecmp(var, name, len)==0) break;
		
	if(var) {
		line=strchr(var, ' ');
		if(line) {
			line++;
			ret=line;
			var=h_ead->next();
			while(var) {
				if(var[0]==32 || var[0]==9) {
					ret+=var;
					var=h_ead->next();
				}	
				else break;				
			}
		}
		/*pos=ret.find(' ');
		if(pos!=-1) ret.remove(0, pos+1);
		else ret="";*/
	}
	
	return ret;
}



DwString KNMimeContent::decodedData()
{
	DwString src, dst;
	DwUuencode dwuu;
		
	if(mInfo->ctMediaType()==MTtext && mInfo->isReadable()) {
		dst="";
		for(char *line=firstBodyLine(); line; line=nextBodyLine()) {
			dst+=line;
			dst+="\n";
		}
	}
	else {
		src="";
		for(char *line=firstBodyLine(); line; line=nextBodyLine()) {
			src+=line;
			src+="\r\n";
		}	
	
		switch(mInfo->ctEncoding()) {
			case ECquotedPrintable:
				DwDecodeQuotedPrintable(src, dst);
			break;
			case ECbase64:
				DwDecodeBase64(src, dst);
			break;
			case ECuuencode:
				qDebug("uudecode");
				dwuu.SetAsciiChars(src);
				dwuu.Decode();
				dst=dwuu.BinaryChars();	
			break;
			default:
				dst=src;
			break;
		}
	}
	
	return dst;
}



DwString KNMimeContent::encodedData()
{
	char *line;
	QCString boundary;
	DwString data, dst, src;
	data="";
		
	for(line=h_ead->first(); line; line=h_ead->next()) {
		if(strncasecmp(line, "X-KNode", 7)==0) continue;
		data+=line;
		data+="\r\n";
	}
	data+="\r\n";
	
	if(b_ody) {
		if(	mInfo->ctMediaType()==MTtext &&
				(mInfo->ctEncoding()==ECquotedPrintable || mInfo->ctEncoding()==ECbase64) &&
				mInfo->isReadable()	) {
			
			src="";
			for(line=b_ody->first(); line; line=b_ody->next()) {
				src+=line;
				src+="\r\n";
			}
			if(mInfo->ctEncoding()==ECquotedPrintable) DwEncodeQuotedPrintable(src, dst);
			else DwEncodeBase64(src, dst);
			data+=dst;
		}				
		else {
			for(line=b_ody->first(); line; line=b_ody->next()) {
				data+=line;
				data+="\r\n";
			}
		}	
	}
	else if(ct_List) {
		boundary=mimeInfo()->getCTParameter("boundary");
		for(KNMimeContent *con=ct_List->first(); con; con=ct_List->next()) {
	  	data+="--";
	  	data+=boundary.data();
	  	data+="\r\n";
	  	data+=con->encodedData();
	  	data+="\r\n";
	  }
	  data+="--";
	  data+=boundary.data();
	  data+="--\r\n";
	}
	
	return data;	  	
}



void KNMimeContent::toStream(QTextStream &ts)
{
	char *line;
	QCString boundary, decodedText, tmp;
	
	if(b_ody==0 && ct_List!=0) {
		boundary=mimeInfo()->getCTParameter("boundary");
		if(boundary.isEmpty()) {
			qDebug("KNMimeContent::toStream() : no boundary found - creating new one!!");
			boundary=multiPartBoundary();
			tmp="boundary=\""+boundary+"\"";
			mimeInfo()->addCTParameter(tmp);
		}
	}	
	
	for(line=h_ead->first(); line; line=h_ead->next())
	  if(strncasecmp(line, "X-KNode-Tempfile", 16)==0) continue; //temporary path is not saved
	  else ts << line << '\n';
	
	ts << '\n';
	
	if(b_ody) {
		if(	mInfo->ctMediaType()==MTtext &&
				(mInfo->ctEncoding()==ECquotedPrintable || mInfo->ctEncoding()==ECbase64) &&
				mInfo->isReadable()	) {
			decodedText="";
			for(line=b_ody->first(); line; line=b_ody->next()) {
				decodedText+=line;
				decodedText+="\n";
			}
			if(mInfo->ctEncoding()==ECquotedPrintable)
				ts << encodeQuotedPrintable(decodedText);
			else
				ts << encodeBase64(decodedText);
		}				
		else {
			for(line=b_ody->first(); line; line=b_ody->next())
			  ts << line << '\n';
		}	
	}
	else if(ct_List) {
		for(KNMimeContent *con=ct_List->first(); con; con=ct_List->next()) {
		  ts << "--" << boundary << '\n';
	  	con->toStream(ts);
	  	ts << '\n';
	  }
	  ts << "--" << boundary << "--\n";
	}	  	
}

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
#include <qfileinfo.h>
#include <qtextcodec.h>

#include <mimelib/string.h>
#include <mimelib/utility.h>
#include <mimelib/uuencode.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmimemagic.h>
#include <kcharsets.h>


#include "knarticlecollection.h"
#include "knstringsplitter.h"
#include "knmimecontent.h"
#include "utilities.h"
#include "knglobals.h"
#include "knode.h"


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
  
  if(ct_List) {
    delete ct_List;
    ct_List=0;
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



// Attention: this method is called from the network thread!
void KNMimeContent::parse()
{
  QCString tmp;
  KNMimeContent *mCT=0, *uuContent=0;
  QStrList *part;
  contentCategory cat=CCsingle;
      
  //parse Header
  mimeInfo()->parse(this);

  //parse Body
  if(b_ody && mInfo->ctMediaType()==MTmultipart) {
    mInfo->setCTCategory(CCcontainer);
    if(mInfo->ctSubType()==STalternative) cat=CCalternativePart;
    else cat=CCmixedPart;
    
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
        mCT->mimeInfo()->setCTCategory(cat);
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
  else if(b_ody && mInfo->ctMediaType()==MTtext && b_ody->count()>200) {
    qDebug("knode: KNMimeContent::parse() : uuencoded binary assumed");
    UUParser uup(b_ody, headerLine("Subject"));
    uup.parse();
    
    if(uup.isPartial()) {
      mInfo->setCTMediaType(MTmessage);
      mInfo->setCTSubType(STpartial);
      mInfo->setCTEncoding(ECsevenBit);
      mInfo->setCTParameter("id", uniqueString());
      tmp.setNum(uup.numberOfPart());
      mInfo->setCTParameter("number", tmp, false);
      tmp.setNum(uup.totalNumberOfParts());
      mInfo->setCTParameter("total", tmp, false);
      this->KNMimeContent::assemble();
    }
    
    else if(uup.isUUencoded()) {
      qDebug("knode: KNMimeContent::parse() : is uuencoded");
      if(!ct_List) {
        ct_List=new QList<KNMimeContent>;
        ct_List->setAutoDelete(true);
      }
      else ct_List->clear();
      mInfo->setCTCategory(CCcontainer);
      mInfo->setCTMediaType(MTmultipart);
      mInfo->setCTSubType(STmixed);
      mInfo->setCTEncoding(ECnone);
      mInfo->setBoundaryParameter(multiPartBoundary());
      this->KNMimeContent::assemble();
      
      if((part=uup.textPart())) {
        uuContent=new KNMimeContent();
        uuContent->initContent();
        uuContent->mimeInfo()->setCTMediaType(MTtext);
        uuContent->mimeInfo()->setCTSubType(STplain);
        uuContent->mimeInfo()->setCTEncoding(ECsevenBit);
        uuContent->mimeInfo()->setCTDisposition(DPinline);
        uuContent->mimeInfo()->setCTCategory(CCmixedPart);
        uuContent->assemble();
        for(char *l=part->first(); l; l=part->next())
          uuContent->addBodyLine(l);
        ct_List->append(uuContent);
      }
      
      part=uup.binaryPart();  
      uuContent=new KNMimeContent();
      uuContent->initContent();
      tmp=uup.assumedMimeType().copy()+"; name=\""+uup.fileName()+"\"";
      uuContent->mimeInfo()->setCustomMimeType(tmp);
      uuContent->mimeInfo()->setCTEncoding(ECuuencode);
      uuContent->mimeInfo()->setCTDisposition(DPattached);
      uuContent->mimeInfo()->setCTCategory(CCmixedPart);
      uuContent->assemble();
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
    if(!tmp.isEmpty()) {
      if(i->ctDisposition()==DPattached)
        tmp+="; filename=\""+i->getCTParameter("name")+"\"";
      setHeader(HTdisposition, tmp, false);
    }
    else
      removeHeader("Content-Disposition");
  }

  tmp=i->contentTransferEncoding();

  if(!tmp.isEmpty())
    setHeader(HTencoding, tmp, false);
  else
    removeHeader("Content-Transfer-Encoding");

  if(i->ctMediaType()==MTtext && !i->ctParameterIsSet("charset"))
    i->setCharsetParameter(defaultChSet);

  tmp=i->contentType();
  if(!tmp.isEmpty())
      setHeader(HTcontentType, tmp, false);
  else
    removeHeader("Content-Type");

  if(ct_List)
    for(KNMimeContent *var=ct_List->first(); var; var=ct_List->next())
      var->assemble();
}



int KNMimeContent::contentSize()
{
  int siz=0;

  if(b_ody)
    for(char *line=b_ody->first(); line; line=b_ody->next())
      siz+=strlen(line);
  else if(ct_List)
    for(KNMimeContent *c=ct_List->first(); c; c=ct_List->next())
      siz+=c->contentSize();

  return siz;
}



int KNMimeContent::contentLineCount()
{
  int cnt=0;

  if(b_ody)
    cnt=b_ody->count();
  else if(ct_List)
    for(KNMimeContent *c=ct_List->first(); c; c=ct_List->next())
      cnt+=c->contentLineCount();

  return cnt;
}



KNMimeInfo* KNMimeContent::mimeInfo()
{
  if(!mInfo) mInfo=new KNMimeInfo();
  return mInfo;
}



void KNMimeContent::copyContent(KNMimeContent *c)
{
  KNMimeContent *content;
  initContent();
  *(this->mInfo) = *(c->mInfo);

  *(this->h_ead) = *(c->h_ead);

  if(c->b_ody) {
    *(this->b_ody) = *(c->b_ody);
  }
  else if(c->ct_List) {
    ct_List=new QList<KNMimeContent>;
    ct_List->setAutoDelete(true);

    for(KNMimeContent *var=c->ct_List->first(); var; var=c->ct_List->next()) {
      content=new KNMimeContent();
      content->copyContent(var);
      ct_List->append(content);
    }
  }

}



void KNMimeContent::decodeText()
{
  
  if(mInfo->decoded()) return;
    
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
    
    mInfo->setDecoded(true);
  }
}



void KNMimeContent::changeEncoding(int e)
{

  if(mimeInfo()->ctEncoding()==e)
    return;

  if(mimeInfo()->ctMediaType()==MTtext) {
    //if(!mimeInfo()->decoded()) // content may be encoded so we decode it first
      decodeText();
    mimeInfo()->setCTEncoding((encoding)e); // textual data is not encoded until it's sent or saved so we just set the new encoding
  }
  else {

    if(e!=ECbase64) {
      kdWarning(5003) << "KNMimeContent::changeEncoding() : non textual data and encoding != base64 - this should not happen => forcing base64" << endl;
      e=ECbase64;
    }
    
    if(mimeInfo()->ctEncoding()!=ECbase64) {
      DwString src, dst;
      QCString tmp;
      KNStringSplitter split;
      bool splitOk=false;
      src=decodedData(); // decode
      DwEncodeBase64(src, dst); //encode as base64
      b_ody->clear();
      tmp=dst.c_str();
      split.init(tmp, "\n");
      splitOk=split.first();
      if(!splitOk)
        b_ody->append(tmp.data());
      else {
        while(splitOk) {
          b_ody->append(split.string().data());
          splitOk=split.next();
        }
      }
      mimeInfo()->setDecoded(false);
      mimeInfo()->setCTEncoding(ECbase64);
    }
  }
}



KNMimeContent* KNMimeContent::textContent()
{
  KNMimeContent *ret=0;

  if(mimeInfo()->ctMediaType()==MTtext && mimeInfo()->ctDisposition()==DPinline)
    ret=this;
  else if(ct_List) {
    for(KNMimeContent *var=ct_List->first(); var; var=ct_List->next()) {
      ret=var->textContent();
      if(ret) break;
    }
  }
  return ret;
}



QCString KNMimeContent::htmlCode()
{
  QCString html, tmp;
  int htmlPos1=-1,
      htmlPos2=-1,
      bodyPos1=-1,
      bodyPos2=-1,
      pos=0;

  if(b_ody) {
    for(char *line=b_ody->first(); line; line=b_ody->next()) {

      if(htmlPos1==-1 && (strstr(line, "<html>") || strstr(line, "<HTML>")))
        htmlPos1=b_ody->at();
      else if(htmlPos2==-1 && (strstr(line, "</html>") || strstr(line, "</HTML>")))
        htmlPos2=b_ody->at();
      else if(bodyPos1==-1 && (strstr(line, "<body") || strstr(line, "<BODY")))
        bodyPos1=b_ody->at();
      else if(bodyPos2==-1 && (strstr(line, "</body>") || strstr(line, "</BODY>")))
        bodyPos2=b_ody->at();
    }

    for(int idx=0; idx < (int)(b_ody->count()); idx++) {

      if(idx<htmlPos1 || idx<bodyPos1)
        continue;

      if(idx==htmlPos1 && bodyPos1==-1) {
        tmp=b_ody->at(idx);
        pos=tmp.find("<html>", 0, false)+6;
        tmp=tmp.right(tmp.length()-pos);
        if(!tmp.isEmpty())
          html+=tmp;
      }
      else if(idx==bodyPos1) {
        tmp=b_ody->at(idx);
        pos=tmp.find("<body", 0, false)+6;
        pos=tmp.find('>', pos)+1;
        if(pos!=-1) {
          tmp=tmp.right(tmp.length()-pos);
          if(!tmp.isEmpty())
            html+=tmp;
        }
      }
      else if(idx==bodyPos2) {
        tmp=b_ody->at(idx);
        pos=tmp.find("</body>", 0, false);
        tmp=tmp.left(pos);
        if(!tmp.isEmpty())
          html+=tmp;
        break;
      }
      else if(idx==htmlPos2) {
        tmp=b_ody->at(idx);
        pos=tmp.find("</html>", 0, false);
        tmp=tmp.left(pos);
        if(!tmp.isEmpty())
          html+=tmp;
        break;
      }
      else
        html+=b_ody->at(idx);
    }
  }

  return html;
}



void KNMimeContent::attachments(QList<KNMimeContent> *dst, bool incAlternatives)
{
  if(!ct_List) {
    dst->append(this);
  }
  else {
    KNMimeContent *textCT=textContent();
    
    for(KNMimeContent *var=ct_List->first(); var; var=ct_List->next()) {
      if(var==textCT || (!incAlternatives && var->mimeInfo()->ctCategory()==CCalternativePart) )
        continue;
      else
        var->attachments(dst, incAlternatives);
    }
  }
}



QCString KNMimeContent::ctCharset()
{
  QCString ret;
  ret=mimeInfo()->getCTParameter("charset").upper();
  if (ret.isEmpty() || (ret=="US-ASCII"))
    ret="ISO-8859-1";
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



QCString KNMimeContent::ctEncoding()
{
  QCString ret;

  ret=headerLine("Content-Transfer-Encoding");
  if(ret.isEmpty())
    ret=mimeInfo()->contentTransferEncoding();

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
  ret=decodeRFC1522String(headerLine("Content-Description"));

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
  headerType ht;
  QCString hdr;
  
  for(line=h_ead->first(); line; line=h_ead->next()) {
    if (line[0]!=32 && line[0]!=9) {   // don't insert into continued headers
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
    } else insPos=h_ead->at()+1;
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
      


QCString KNMimeContent::headerLine(const char* name, bool decode)
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

    if(decode && !ret.isEmpty())
      ret=decodeRFC1522String(ret);
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
        kdDebug(5003) << "uudecode" << endl;
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
    if( mInfo->ctMediaType()==MTtext &&
        (mInfo->ctEncoding()==ECquotedPrintable || mInfo->ctEncoding()==ECbase64) &&
        mInfo->isReadable() ) {
      if(mInfo->ctEncoding()==ECquotedPrintable) {
        for(line=b_ody->first(); line; line=b_ody->next()) {
          src=line;
          DwEncodeQuotedPrintable(src, dst);
          data+=dst+"\r\n";
        }
      } else {         // Base64 (bad idea)
        src="";
        for(line=b_ody->first(); line; line=b_ody->next()) {
          src+=line;
          src+="\r\n";
        }
        DwEncodeBase64(src, dst);
        data+=dst;
      }
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
      
  for(line=h_ead->first(); line; line=h_ead->next())
    if(strncasecmp(line, "X-KNode-Tempfile", 16)==0) continue; //temporary path is not saved
    else ts << line << '\n';
  
  ts << '\n';
  
  if(b_ody) {
    if( mInfo->ctMediaType()==MTtext &&
        (mInfo->ctEncoding()==ECquotedPrintable || mInfo->ctEncoding()==ECbase64) &&
        mInfo->isReadable() ) {
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
    boundary=mimeInfo()->getCTParameter("boundary");
    for(KNMimeContent *con=ct_List->first(); con; con=ct_List->next()) {
      ts << "--" << boundary << '\n';
      con->toStream(ts);
      ts << '\n';
    }
    ts << "--" << boundary << "--\n";
  }     
}



void KNMimeContent::addContent(KNMimeContent *c, bool prepend)
{
  if(!ct_List) {
    ct_List=new QList<KNMimeContent>;
    ct_List->setAutoDelete(true);
    if(b_ody && b_ody->count() > 0) {
      KNMimeContent * main=new KNMimeContent();
      main->initContent();
      *(main->b_ody) = *(this->b_ody);
      *(main->mimeInfo()) = *(this->mimeInfo());
      ct_List->append(main);
    }
    delete b_ody;
    b_ody=0;
    mimeInfo()->clear();
    mimeInfo()->setCTMediaType(MTmultipart);
    mimeInfo()->setCTSubType(STmixed);
    mimeInfo()->setDecoded(false);
    mimeInfo()->setCTCategory(CCcontainer);
    mimeInfo()->setCTEncoding(ECnone);
    mimeInfo()->setBoundaryParameter(multiPartBoundary());
  }

  if(prepend)
    ct_List->insert(0, c);
  else
    ct_List->append(c);
}



void KNMimeContent::removeContent(KNMimeContent *c, bool del)
{
  int idx=0;
  KNMimeContent *main=0;

  if(del)
    ct_List->removeRef(c);
  else {
    idx=ct_List->findRef(c);
    ct_List->take(idx);
  }

  if(ct_List->count()==1) {
    main=ct_List->first();
    if(!b_ody) {
      b_ody=new QStrList();
      b_ody->setAutoDelete(true);
    }

    *(this->b_ody) = *(main->b_ody);
    *(this->mimeInfo()) = *(main->mimeInfo());

    delete ct_List;
    ct_List=0;
  }
}



//=============================================================================================



KNContentCodec::KNContentCodec(KNMimeContent *c)
{
  setSourceContent(c);
}



KNContentCodec::~KNContentCodec()
{
}



bool KNContentCodec::setFirstLine()
{
  if(s_rc)
    l_ine=s_rc->firstBodyLine();
  else
    l_ine=0;

  return (l_ine!=0);
}



bool KNContentCodec::setNextLine()
{
  if(s_rc)
    l_ine=s_rc->nextBodyLine();
  else
    l_ine=0;

  return (l_ine!=0);
}



QString KNContentCodec::currentUnicodeLine()
{
  return(toUnicode(l_ine));
}


QString KNContentCodec::asUnicodeString()
{
  QString ret;

  if(s_rc->mimeInfo()->ctSubType()==KNArticleBase::SThtml)
    ret=toUnicode(s_rc->htmlCode());
  else
    for(l_ine=s_rc->firstBodyLine(); l_ine; l_ine=s_rc->nextBodyLine())
      ret+=toUnicode(l_ine)+"\n";

  return ret;
}



void KNContentCodec::matchFont(QFont &f)
{
  KCharsets *c=KGlobal::charsets();

  if(c_sAvailable)
    c->setQFont(f, c_harset);
  else
    c->setQFont(f, c->charsetForLocale());
}



void KNContentCodec::setSourceContent(KNMimeContent *c)
{
  s_rc=c;
  l_ine=0;

  if(!c) {
    c_harset=QString::null;
    c_odec=0;
    c_sAvailable=false;
  }
  else
    setCharset(QString(c->ctCharset()));
}



void KNContentCodec::setCharset(const QString &chset)
{
  if(chset==QString::null) {
    kdDebug(5003) << "void KNContentCodec::setCharset() : charset cannot be set to QString::null !! => returning" << endl;
    return;
  }

  c_harset=chset;

  KCharsets *c=KGlobal::charsets();

  if(!(c_sAvailable=c->isAvailable(c_harset)))
    c_odec=0;
  else
    c_odec=c->codecForName(c_harset);
}



/*void KNContentCodec::fromUnicodeString(const QString &unicode)
{
  KNStringSplitter split;
  bool splitOK;
  split.init(fromUnicode(unicode), "\n");

  s_rc->clearBody();

  if(!(splitOK=split.first()))
    s_rc->addBodyLine(split.source());
  else
    while(splitOK) {
      s_rc->addBodyLine(split.string());
      splitOK=split.next();
    }
}



void KNContentCodec::appendLine(const QString &l)
{
  s_rc->addBodyLine(fromUnicode(l));
}*/



QString KNContentCodec::toUnicode(const char *aStr)
{
  if(!aStr) {
    kdDebug(5003) << "KNContentCodec::toUnicode() : aStr==0 this should not happen => returnin QString::null" << endl;
    return QString::null;
  }

  QString uc;

  if(c_odec)
    uc=c_odec->toUnicode(aStr, strlen(aStr));
  else {
    kdDebug(5003) << "KNContentCodec::toUnicode() : no codec available!! => Text is not converted" << endl;
    uc=QString::fromLatin1(aStr); //take the text "as is" and hope the best ;-)
  }

  return uc;
}


/*QCString KNContentCodec::fromUnicode(const QString &uc)
{
  if(!c_odec) {
    kdDebug(5003) << "KNContentCodec::fromUnicode() : no codec available!! => using local charset instead" << endl;
    KCharsets *c=KGlobal::charsets();
    setCharset(c->name(c->charsetForLocale()));
    s_rc->mimeInfo()->setCharsetParameter(QCString(c_harset.latin1()));
  }

  return c_odec->fromUnicode(uc);
}*/



//=============================================================================================



KNAttachment::KNAttachment(KNMimeContent *c)
  : c_ontent(c), i_sAttached(true), h_asChanged(false)
{
  c_tName=c->ctName();
  setContentMimeType(c->ctMimeType());
  c_tDescription=c->ctDescription();
  c_te=c->mimeInfo()->ctEncoding();
}




KNAttachment::KNAttachment(const QString &path)
  : c_ontent(0), i_sAttached(false), h_asChanged(true)
{
  setContentMimeType((KMimeMagic::self()->findFileType(path))->mimeType());
  f_ile.setName(path);
  c_tName=QFileInfo(f_ile).fileName();
}



KNAttachment::~KNAttachment()
{
  if(!i_sAttached && c_ontent)
    delete c_ontent;
}



QString KNAttachment::contentEncoding()
{
  QString e=KNArticleBase::encodingToString(c_te);

  return e;
}



QString KNAttachment::contentSize()
{
  QString ret;
  int s=0;

  if(c_ontent && c_ontent->hasContent())
    s=c_ontent->contentSize();
  else
    s=f_ile.size();
  if(s > 1023) {
    s=s/1024;
    ret.setNum(s);
    ret+="kB";
  }
  else {
    ret.setNum(s);
    ret+="Bytes";
  }

  return ret;
}



void KNAttachment::setContentMimeType(const QString &s)
{
  c_tMimeType=s;
  h_asChanged=true;

  if(s.find("text/", 0, false)==-1) {
    f_b64=true;
    c_te=KNArticleBase::ECbase64;
  }
  else {
    f_b64=false;
    c_te=KNArticleBase::defaultTextEncoding();
  }
}



void KNAttachment::updateContentInfo()
{
  if(h_asChanged) {

    if(c_tDescription.isEmpty())
      c_ontent->removeHeader("Content-Description");
    else
      c_ontent->setHeader(KNArticleBase::HTdescription, c_tDescription.local8Bit(), true);

    c_ontent->mimeInfo()->setCustomMimeType(c_tMimeType.local8Bit());

    if(i_sAttached && c_te!=c_ontent->mimeInfo()->ctEncoding())
      c_ontent->changeEncoding(c_te);
    else
      c_ontent->mimeInfo()->setCTEncoding(c_te);
  }
}



void KNAttachment::attach(KNMimeContent *c)
{

  if(!i_sAttached && !c_ontent && !f_ile.name().isEmpty()) {

    if(!f_ile.open(IO_ReadOnly)) {
      displayExternalFileError();
      i_sAttached=false;
      return;
    }

    c_ontent=new KNMimeContent();
    c_ontent->initContent();
    c_ontent->mimeInfo()->setCTDisposition(KNArticleBase::DPattached);
    c_ontent->mimeInfo()->setCTCategory(KNArticleBase::CCmixedPart);
    c_ontent->mimeInfo()->setCustomMimeType(c_tMimeType.local8Bit());
    c_ontent->mimeInfo()->setNameParameter(c_tName.local8Bit());
    updateContentInfo();



    if(c_te==KNArticleBase::ECbase64 && c_ontent->mimeInfo()->ctMediaType()!=KNArticleBase::MTtext) { //encode base64

      char *buff=new char[5700];
      int readBytes=0;
      bool splitOk=false;
      KNStringSplitter split;
      DwString dest;
      DwString src;
      QCString data;

      while(!f_ile.atEnd()) {
        // read 5700 bytes at once :
        // 76 chars per line * 6 bit per char / 8 bit per byte => 57 bytes per line
        // append 100 lines in a row => encode 5700 bytes
        readBytes=f_ile.readBlock(buff, 5700);
        src.assign(buff, readBytes);
        DwEncodeBase64(src, dest);
        data=dest.c_str();
        split.init(data, "\n");
        splitOk=split.first();
        while(splitOk) {
          if(!split.string().isEmpty())
            c_ontent->addBodyLine(split.string().data());
          splitOk=split.next();
        }
        //kapp->processEvents();
      }

      f_ile.close();
      delete[] buff;
      c_ontent->mimeInfo()->setDecoded(false);
    }
    else { //do not encode text
      QCString data;
      while(!f_ile.atEnd()) {
        data=f_ile.readLine();
        if(f_ile.status()==IO_Ok)
          c_ontent->addBodyLine(data);
        else {
          displayExternalFileError();
          c_ontent->clearBody();
          break;
        }
      }

      c_ontent->mimeInfo()->setDecoded(true);
    }

    if(c_ontent->hasContent()) {
      c->addContent(c_ontent);
      i_sAttached=true;
    }
  }
}



void KNAttachment::detach(KNMimeContent *c)
{
  if(i_sAttached) {
    c->removeContent(c_ontent, false);
    i_sAttached=false;
  }
}


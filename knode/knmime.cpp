/*
    knmime.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include <ctype.h>
#include <mimelib/string.h>
#include <mimelib/utility.h>
#include <mimelib/uuencode.h>
#include <stdlib.h>
#include <unistd.h>

#include <qtextcodec.h>
#include <qfileinfo.h>
#include <qstringlist.h>

#include <kglobal.h>
#include <klocale.h>
#include <kmdcodec.h>
#include <kcharsets.h>
#include <kmimemagic.h>
#include <kdebug.h>

#include "knmime.h"
#include "knhdrviewitem.h"
#include "kngroup.h"
#include "knglobals.h"
#include "knconfigmanager.h"
#include "utilities.h"


QString KNMimeBase::decodeRFC2047String(const QCString &src, const char **usedCS, const QCString &defaultCS, bool forceCS)
{
  QCString result, str;
  QCString declaredCS;
  DwString dwsrc, dwdest;
  char *pos, *dest, *beg, *end, *mid;
  char encoding, ch;
  bool valid;
  const int maxLen=400;
  int i;

  if(src.find("=?") < 0)
    result = src.copy();
  else {
    result.truncate(src.length());
    for (pos=src.data(), dest=result.data(); *pos; pos++)
    {
      if (pos[0]!='=' || pos[1]!='?')
      {
        *dest++ = *pos;
        continue;
      }
      beg = pos+2;
      end = beg;
      valid = TRUE;
      // parse charset name
      declaredCS="";
      for (i=2,pos+=2; i<maxLen && (*pos!='?'&&(ispunct(*pos)||isalnum(*pos))); i++) {
        declaredCS+=(*pos);
        pos++;
      }
      if (*pos!='?' || i<4 || i>=maxLen) valid = FALSE;
      else
      {
        // get encoding and check delimiting question marks
        encoding = toupper(pos[1]);
        if (pos[2]!='?' || (encoding!='Q' && encoding!='B'))
          valid = FALSE;
        pos+=3;
        i+=3;
      }
      if (valid)
      {
        mid = pos;
        // search for end of encoded part
        while (i<maxLen && *pos && !(*pos=='?' && *(pos+1)=='='))
        {
          i++;
          pos++;
        }
        end = pos+2;//end now points to the first char after the encoded string
        if (i>=maxLen || !*pos) valid = FALSE;
      }

      if (valid) {
        ch = *pos;
        *pos = '\0';
        str = QCString(mid, (int)(mid - pos - 1));
        if (encoding == 'Q')
        {
          // decode quoted printable text
          for (i=str.length()-1; i>=0; i--)
            if (str[i]=='_') str[i]=' ';
          dwsrc=str.data();
          DwDecodeQuotedPrintable(dwsrc, dwdest);
          str = dwdest.c_str();
        }
        else
        {
          // decode base64 text
          dwsrc=str.data();
          DwDecodeBase64(dwsrc, dwdest);
          str = dwdest.c_str();
        }
        *pos = ch;
        for (i=0; str[i]; i++)
        *dest++ = str[i];

        pos = end -1;
      }
      else
      {
        //result += "=?";
        //pos = beg -1; // because pos gets increased shortly afterwards
        pos = beg - 2;
        *dest++ = *pos++;
        *dest++ = *pos;
      }
    }
    *dest = '\0';
  }

  //find suitable QTextCodec
  QTextCodec *codec=0;
  bool ok=true;
  if (forceCS || declaredCS.isEmpty()) {
    codec=KGlobal::charsets()->codecForName(defaultCS);
    (*usedCS)=cachedCharset(defaultCS);
  }
  else {
    codec=KGlobal::charsets()->codecForName(declaredCS, ok);
    if(!ok) {     //no suitable codec found => use default charset
      codec=KGlobal::charsets()->codecForName(defaultCS);
      (*usedCS)=cachedCharset(defaultCS);
    }
    else
      (*usedCS)=cachedCharset(declaredCS);
  }

  return codec->toUnicode(result.data(), result.length());
}


QCString KNMimeBase::encodeRFC2047String(const QString &src, const char *charset, bool addressHeader)
{
  QCString encoded8Bit, result, usedCS;
  unsigned int start=0,end=0;
  bool nonAscii=false, ok=true, useQEncoding=false;
  QTextCodec *codec=0;
  KNConfig::PostNewsTechnical *pnt=knGlobals.cfgManager->postNewsTechnical();

  usedCS=charset;
  codec=KGlobal::charsets()->codecForName(usedCS, ok);

  if(!ok) {
    //no codec available => try local8Bit and hope the best ;-)
    usedCS=KGlobal::locale()->charset().latin1();
    codec=KGlobal::charsets()->codecForName(usedCS, ok);
  }

  if (usedCS.find("8859-")>=0)  // use "B"-Encoding for non iso-8859-x charsets
    useQEncoding=true;

  encoded8Bit=codec->fromUnicode(src);

  if(pnt->allow8BitHeaders())
    return encoded8Bit;

  for (unsigned int i=0; i<encoded8Bit.length(); i++) {
    if (encoded8Bit[i]==' ')    // encoding starts at word boundaries
      start = i+1;

    // encode escape character, for japanese encodings...
    if ((encoded8Bit[i]<0) || (encoded8Bit[i] == '\033') ||
        (addressHeader && (strchr("\"()<>@,;:\\[]=",encoded8Bit[i])!=0))) {
      end = start;   // non us-ascii char found, now we determine where to stop encoding
      nonAscii=true;
      break;
    }
  }

  if (nonAscii) {
    while ((end<encoded8Bit.length())&&(encoded8Bit[end]!=' '))  // we encode complete words
      end++;

    for (unsigned int x=end;x<encoded8Bit.length();x++)
      if ((encoded8Bit[x]<0) || (encoded8Bit[x] == '\033') ||
          (addressHeader && (strchr("\"()<>@,;:\\[]=",encoded8Bit[x])!=0))) {
        end = encoded8Bit.length();     // we found another non-ascii word

      while ((end<encoded8Bit.length())&&(encoded8Bit[end]!=' '))  // we encode complete words
        end++;
    }

    result = encoded8Bit.left(start)+"=?"+usedCS;

    if (useQEncoding) {
      result += "?Q?";

      char c,hexcode;                       // implementation of the "Q"-encoding described in RFC 2047
      for (unsigned int i=start;i<end;i++) {
        c = encoded8Bit[i];
        if (c == ' ')       // make the result readable with not MIME-capable readers
          result+='_';
        else
          if (((c>='a')&&(c<='z'))||      // paranoid mode, we encode *all* special characters to avoid problems
              ((c>='A')&&(c<='Z'))||      // with "From" & "To" headers
              ((c>='0')&&(c<='9')))
            result+=c;
          else {
            result += "=";                 // "stolen" from KMail ;-)
            hexcode = ((c & 0xF0) >> 4) + 48;
            if (hexcode >= 58) hexcode += 7;
            result += hexcode;
            hexcode = (c & 0x0F) + 48;
            if (hexcode >= 58) hexcode += 7;
            result += hexcode;
          }
      }
    } else {
      result += "?B?"+QCString(KCodecs::base64Encode(QString::fromLatin1(encoded8Bit.mid(start,end-start).data())).latin1());
    }

    result +="?=";
    result += encoded8Bit.right(encoded8Bit.length()-end);
  }
  else
    result = encoded8Bit;

  return result;
}


QStrIList KNMimeBase::c_harsetCache;

const char* KNMimeBase::cachedCharset(const QCString &name)
{
  int idx=c_harsetCache.find(name.data());
  if(idx>-1)
    return c_harsetCache.at(idx);

  c_harsetCache.append(name.upper().data());
  //qDebug("KNMimeBase::cachedCharset() number of cs %d", c_harsetCache.count());
  return c_harsetCache.last();
}


QCString KNMimeBase::uniqueString()
{
  static char chars[] = "0123456789abcdefghijklmnopqrstuvxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  time_t now;
  QCString ret;
  char p[11];
  int pos, ran;
  unsigned int timeval;

  p[10]='\0';
  now=time(0);
  ran=1+(int) (1000.0*rand()/(RAND_MAX+1.0));
  timeval=(now/ran)+getpid();

  for(int i=0; i<10; i++){
    pos=(int) (61.0*rand()/(RAND_MAX+1.0));
    //kdDebug(5003) << pos << endl;
    p[i]=chars[pos];
  }
  ret.sprintf("%d.%s", timeval, p);

  return ret;
}


QCString KNMimeBase::multiPartBoundary()
{
  QCString ret;
  ret="nextPart"+uniqueString();
  return ret;
}


QCString KNMimeBase::extractHeader(const QCString &src, const char *name)
{
  QCString n=QCString(name)+": ";
  int pos1=-1, pos2=0, len=src.length()-1;
  bool folded(false);

  if (n == src.left(n.length())) {
    pos1 = 0;
  } else {
    n.prepend("\n");
    pos1 = src.find(n);
  }

  if (pos1>-1) {    //there is a header with the given name
    pos1+=n.length(); //skip the name
    pos2=pos1;

    if (src[pos2]!='\n') {  // check if the header is not empty
      while(1) {
        pos2=src.find("\n", pos2+1);
        if(pos2==-1 || pos2==len || ( src[pos2+1]!=' ' && src[pos2+1]!='\t') ) //break if we reach the end of the string, honor folded lines
          break;
        else
          folded = true;
      }
    }

    if(pos2<0) pos2=len+1; //take the rest of the string

    if (!folded)
      return src.mid(pos1, pos2-pos1);
    else
      return (src.mid(pos1, pos2-pos1).replace(QRegExp("\\n\\s")," "));
  }
  else {
    return QCString(""); //header not found
  }
}


QCString KNMimeBase::CRLFtoLF(const QCString &s)
{
  QCString ret=s.copy();
  ret.replace(QRegExp("\\r\\n"), "\n");
  return ret;
}


QCString KNMimeBase::CRLFtoLF(const char *s)
{
  QCString ret=s;
  ret.replace(QRegExp("\\r\\n"), "\n");
  return ret;
}


QCString KNMimeBase::LFtoCRLF(const QCString &s)
{
  QCString ret=s.copy();
  ret.replace(QRegExp("\\n"), "\r\n");
  return ret;
}


void KNMimeBase::stripCRLF(char *str)
{
  int pos=strlen(str)-1;
  while(pos>-1 && (str[pos]!='\n' && str[pos]!='\r') ) pos--;
  if(pos>-1) {
    if(str[pos]=='\n' && (pos-1)>0 && str[pos-1]=='\r')
      pos--;
    str[pos]='\0';
  }
}


void KNMimeBase::removeQuots(QCString &str)
{
  int pos1=0, pos2=0;

  if((pos1=str.find('"'))!=-1)
    if((pos2=str.findRev('"'))!=-1)
      if(pos1<pos2)
        str=str.mid(pos1+1, pos2-pos1-1);
}


void KNMimeBase::removeQuots(QString &str)
{
  int pos1=0, pos2=0;

  if((pos1=str.find('"'))!=-1)
    if((pos2=str.findRev('"'))!=-1)
      if(pos1<pos2)
        str=str.mid(pos1+1, pos2-pos1-1);
}


//============================================================================================


KNMimeBase::MultiPartParser::MultiPartParser(const QCString &src, const QCString &boundary)
{
  s_rc=src;
  b_oundary=boundary;
}


KNMimeBase::MultiPartParser::~MultiPartParser() {}


bool KNMimeBase::MultiPartParser::parse()
{
  QCString b="--"+b_oundary, part;
  int pos1=0, pos2=0, blen=b.length();

  p_arts.clear();

  //find the first valid boundary
  while(1) {
    if( (pos1=s_rc.find(b, pos1))==-1 || pos1==0 || s_rc[pos1-1]=='\n' ) //valid boundary found or no boundary at all
      break;
    pos1+=blen; //boundary found but not valid => skip it;
  }

  if(pos1>-1) {
    pos1+=blen;
    if(s_rc[pos1]=='-' && s_rc[pos1+1]=='-') // the only valid boundary is the end-boundary - this message is *really* broken
      pos1=-1; //we give up
    else if( (pos1-blen)>1 ) //preamble present
      p_reamble=s_rc.left(pos1-blen);
  }


  while(pos1>-1 && pos2>-1) {

    //skip the rest of the line for the first boundary - the message-part starts here
    if( (pos1=s_rc.find('\n', pos1))>-1 ) { //now search the next linebreak
      //now find the next valid boundary
      pos2=++pos1; //pos1 and pos2 point now to the beginning of the next line after the boundary
      while(1) {
        if( (pos2=s_rc.find(b, pos2))==-1 || s_rc[pos2-1]=='\n' ) //valid boundary or no more boundaries found
          break;
        pos2+=blen; //boundary is invalid => skip it;
      }

      if(pos2==-1) { // no more boundaries found
        part=s_rc.mid(pos1, s_rc.length()-pos1); //take the rest of the string
        p_arts.append(part);
        pos1=-1;
        pos2=-1; //break;
      }
      else {
        part=s_rc.mid(pos1, pos2-pos1);
        p_arts.append(part);
        pos2+=blen; //pos2 points now to the first charakter after the boundary
        if(s_rc[pos2]=='-' && s_rc[pos2+1]=='-') { //end-boundary
          pos1=pos2+2; //pos1 points now to the character directly after the end-boundary
          if( (pos1=s_rc.find('\n', pos1))>-1 ) //skipt the rest of this line
            e_pilouge=s_rc.mid(++pos1, s_rc.length()-pos1); //everything after the end-boundary is considered as the epilouge
          pos1=-1;
          pos2=-1; //break
        }
        else {
          pos1=pos2; //the search continues ...
        }
      }
    }
  }

  return (!p_arts.isEmpty());
}


//============================================================================================


KNMimeBase::UUParser::UUParser(const QCString &src, const QCString &subject) :
  s_rc(src), s_ubject(subject), p_artNr(-1), t_otalNr(-1)
{}


KNMimeBase::UUParser::~UUParser()
{}


bool KNMimeBase::UUParser::parse()
{
  int currentPos=0;
  bool success=true, firstIteration=true;

  while (success) {
    int beginPos=currentPos, uuStart=currentPos, endPos=0, lineCount=0, MCount=0, pos=0, len=0;
    bool containsBegin=false, containsEnd=false;
    QCString tmp,fileName,mimeType;

    if( (beginPos=s_rc.find(QRegExp("begin [0-9][0-9][0-9]"),currentPos))>-1 && (beginPos==0 || s_rc.at(beginPos-1)=='\n') ) {
      containsBegin=true;
      uuStart=s_rc.find('\n', beginPos);
      if(uuStart==-1) {//no more line breaks found, we give up
        success = false;
        break;
      } else
        uuStart++; //points now at the beginning of the next line
    }
      else beginPos=currentPos;

    if ( (endPos=s_rc.find("\nend",(uuStart>0)? uuStart-1:0))==-1 )
      endPos=s_rc.length(); //no end found
    else
      containsEnd=true;

    if ((containsBegin && containsEnd) || firstIteration) {

      //printf("beginPos=%d , uuStart=%d , endPos=%d\n", beginPos, uuStart, endPos);
      //all lines in a uuencoded text start with 'M'
      for(int idx=uuStart; idx<endPos; idx++)
        if(s_rc[idx]=='\n') {
          lineCount++;
          if(idx+1<endPos && s_rc[idx+1]=='M') {
            idx++;
            MCount++;
          }
        }

      //printf("lineCount=%d , MCount=%d\n", lineCount, MCount);
      if( MCount==0 || (lineCount-MCount)>10 ||
          ((!containsBegin || !containsEnd) && (MCount<15)) ) {  // harder check for splitted-articles
        success = false;
        break; //too many "non-M-Lines" found, we give up
      }

      if( (!containsBegin || !containsEnd) && s_ubject) {  // message may be split up => parse subject
        pos=QRegExp("[0-9]+/[0-9]+").match(QString(s_ubject), 0, &len);
        if(pos!=-1) {
          tmp=s_ubject.mid(pos, len);
          pos=tmp.find('/');
          p_artNr=tmp.left(pos).toInt();
          t_otalNr=tmp.right(tmp.length()-pos-1).toInt();
        } else {
          success = false;
          break; //no "part-numbers" found in the subject, we give up
        }
      }

      //everything before "begin" is text
      if(beginPos>0)
        t_ext.append(s_rc.mid(currentPos,beginPos-currentPos));

      if(containsBegin)
        fileName = s_rc.mid(beginPos+10, uuStart-beginPos-11); //everything between "begin ### " and the next LF is considered as the filename
      else
        fileName = "";
      f_ilenames.append(fileName);
      b_ins.append(s_rc.mid(beginPos, endPos-beginPos+1)); //everything beetween "begin" and "end" is uuencoded

      //try to guess the mimetype from the file-extension
      if(!fileName.isEmpty()) {
        pos=fileName.findRev('.');
        if(pos++ != -1) {
          tmp=fileName.mid(pos, fileName.length()-pos).upper();
          if(tmp=="JPG" || tmp=="JPEG")       mimeType="image/jpeg";
          else if(tmp=="GIF")                 mimeType="image/gif";
          else if(tmp=="PNG")                 mimeType="image/png";
          else if(tmp=="TIFF" || tmp=="TIF")  mimeType="image/tiff";
          else if(tmp=="XPM")                 mimeType="image/x-xpm";
          else if(tmp=="XBM")                 mimeType="image/x-xbm";
          else if(tmp=="BMP")                 mimeType="image/x-bmp";
          else if(tmp=="TXT" ||
                  tmp=="ASC" ||
                  tmp=="H" ||
                  tmp=="C" ||
                  tmp=="CC" ||
                  tmp=="CPP")                 mimeType="text/plain";
          else if(tmp=="HTML" || tmp=="HTM")  mimeType="text/html";
          else                                mimeType="application/octet-stream";
        }
      }
      m_imeTypes.append(mimeType);
      firstIteration=false;

      int next = s_rc.find('\n', endPos+1);
      if(next==-1) { //no more line breaks found, we give up
        success = false;
        break;
      } else
        next++; //points now at the beginning of the next line
      currentPos = next;

    } else {
      success = false;
    }
  }

  // append trailing text part of the article
  t_ext.append(s_rc.right(s_rc.length()-currentPos));

  return ((b_ins.count()>0) || isPartial());
}


//============================================================================================


void KNMimeBase::BoolFlags::set(unsigned int i, bool b)
{
  if(i>15) return;

  unsigned char p; //bitmask
  int n;

  if(i<8) { //first byte
    p=(1 << i);
    n=0;
  }
  else { //second byte
    p=(1 << i-8);
    n=1;
  }

  if(b)
    bits[n] = bits[n] | p;
  else
    bits[n] = bits[n] & (255-p);
}


bool KNMimeBase::BoolFlags::get(unsigned int i)
{
  if(i>15) return false;

  unsigned char p; //bitmask
  int n;

  if(i<8) { //first byte
    p=(1 << i);
    n=0;
  }
  else { //second byte
    p=(1 << i-8);
    n=1;
  }

  return ( (bits[n] & p)>0 );
}


//============================================================================================


KNMimeContent::KNMimeContent()
 : c_ontents(0), h_eaders(0), f_orceDefaultCS(false)
{
  d_efaultCS = cachedCharset("ISO-8859-1");
}


KNMimeContent::KNMimeContent(const QCString &h, const QCString &b)
 : c_ontents(0), h_eaders(0), f_orceDefaultCS(false)
{
  d_efaultCS = cachedCharset("ISO-8859-1");
  h_ead=h.copy();
  b_ody=b.copy();
}


KNMimeContent::~KNMimeContent()
{
  delete c_ontents;
  delete h_eaders;
}


void KNMimeContent::setContent(QStrList *l)
{
  //qDebug("KNMimeContent::setContent(QStrList *l) : start");
  h_ead.resize(0);
  b_ody.resize(0);

  //usage of textstreams is much faster than simply appending the strings
  QTextStream hts(h_ead, IO_WriteOnly),
              bts(b_ody, IO_WriteOnly);
  hts.setEncoding(QTextStream::Latin1);
  bts.setEncoding(QTextStream::Latin1);

  bool isHead=true;
  for(char *line=l->first(); line; line=l->next()) {
    stripCRLF(line); //make sure that there's no newline

    if(isHead && line[0]=='\0') {
      isHead=false;
      continue;
    }
    if(isHead)
      hts << line << "\n";
    else
      bts << line << "\n";
  }

  //terminate strings
  hts << '\0';
  bts << '\0';

  //qDebug("KNMimeContent::setContent(QStrList *l) : finished");
}


void KNMimeContent::setContent(const QCString &s)
{
  int pos=s.find("\n\n", 0);
  if(pos>-1) {
    h_ead=s.left(++pos);  //header *must* end with "\n" !!
    b_ody=s.mid(++pos, s.length()-pos);
  }
  else
    h_ead=s;
}


//parse the message, split multiple parts
void KNMimeContent::parse()
{
  //qDebug("void KNMimeContent::parse() : start");
  delete h_eaders;
  h_eaders=0;
  delete c_ontents;
  c_ontents=0;

  KNHeaders::ContentType *ct=contentType();
  QCString tmp;
  KNMimeContent *c;
  KNHeaders::contentCategory cat;

  // just "text" as mimetype is suspicious, perhaps this article was
  // generated by broken software, better check for uuencoded binaries
  if (ct->mimeType()=="text")
    ct->setMimeType("invalid/invalid");

  if(ct->isText())
    return; //nothing to do

  if(ct->isMultipart()) {   //this is a multipart message
    tmp=ct->boundary(); //get boundary-parameter

    if(!tmp.isEmpty()) {
      MultiPartParser mpp(b_ody, tmp);
      if(mpp.parse()) { //at least one part found

        c_ontents=new List();
        c_ontents->setAutoDelete(true);

        if(ct->isSubtype("alternative")) //examine category for the sub-parts
          cat=KNHeaders::CCalternativePart;
        else
          cat=KNHeaders::CCmixedPart;  //default to "mixed"

        QCStringList parts=mpp.parts();
        QCStringList::Iterator it;
        for(it=parts.begin(); it!=parts.end(); ++it) { //create a new KNMimeContent for every part
          c=new KNMimeContent();
          c->setContent(*it);
          c->parse();
          c->contentType()->setCategory(cat); //set category of the sub-part
          c_ontents->append(c);
          //qDebug("part:\n%s\n\n%s", c->h_ead.data(), c->b_ody.left(100).data());
        }

        //the whole content is now split into single parts, so it's safe delete the message-body
        b_ody.resize(0);
      }
      else { //sh*t, the parsing failed so we have to treat the message as "text/plain" instead
        ct->setMimeType("text/plain");
        ct->setCharset("US-ASCII");
      }
    }
  }
  else if (ct->mimeType()=="invalid/invalid") { //non-mime body => check for uuencoded content
    UUParser uup(b_ody, rawHeader("Subject"));

    if(uup.parse()) { // yep, it is uuencoded

      if(uup.isPartial()) {  // this seems to be only a part of the message so we treat it as "message/partial"
        ct->setMimeType("message/partial");
        //ct->setId(uniqueString()); not needed yet
        ct->setPartialParams(uup.partialCount(), uup.partialNumber());
        contentTransferEncoding()->setCte(KNHeaders::CE7Bit);
      }
      else { //it's a complete message => treat as "multipart/mixed"
        //the whole content is now split into single parts, so it's safe to delete the message-body
        b_ody.resize(0);

        //binary parts
        for (unsigned int i=0;i<uup.binaryParts().count();i++) {
          c=new KNMimeContent();
          //generate content with mime-compliant headers
          tmp="Content-Type: ";
          tmp += uup.mimeTypes().at(i);
          tmp += "; name=\"";
          tmp += uup.filenames().at(i);
          tmp += "\"\nContent-Transfer-Encoding: x-uuencode\nContent-Disposition: attachment; filename=\"";
          tmp += uup.filenames().at(i);
          tmp += "\"\n\n";
          tmp += uup.binaryParts().at(i);
          c->setContent(tmp);
          addContent(c);
        }

        if(c_ontents && c_ontents->first()) { //readd the plain text before the uuencoded part
          c_ontents->first()->setContent("Content-Type: text/plain\nContent-Transfer-Encoding: 7Bit\n\n"+uup.textPart());
          c_ontents->first()->contentType()->setMimeType("text/plain");
        }
      }
    }
    else { //no, this doesn't look like uuencoded stuff => we treat it as "text/plain"
      ct->setMimeType("text/plain");
    }
  }

  //qDebug("void KNMimeContent::parse() : finished");
}


void KNMimeContent::assemble()
{
  QCString newHead="";

  //Content-Type
  newHead+=contentType()->as7BitString()+"\n";

  //Content-Transfer-Encoding
  newHead+=contentTransferEncoding()->as7BitString()+"\n";

  //Content-Description
  KNHeaders::Base *h=contentDescription(false);
  if(h)
    newHead+=h->as7BitString()+"\n";

  //Content-Disposition
  h=contentDisposition(false);
  if(h)
    newHead+=h->as7BitString()+"\n";

  h_ead=newHead;
}


void KNMimeContent::clear()
{
  delete h_eaders;
  h_eaders=0;
  delete c_ontents;
  c_ontents=0;
  h_ead.resize(0);
  b_ody.resize(0);
}


QCString KNMimeContent::encodedContent(bool useCrLf)
{
  QCString e;

  // hack to convert articles with uuencoded binaries into
  // proper mime-compliant articles
  if(c_ontents && !c_ontents->isEmpty()) {
    bool convertFromUunec=false;

    // reencode uuencoded binaries...
    for(KNMimeContent *c=c_ontents->first(); c; c=c_ontents->next()) {
      if (c->contentTransferEncoding(true)->cte()==KNHeaders::CEuuenc) {
        convertFromUunec=true;
        DwUuencode dwuu;
        DwString dwsrc, dwdest;

        if(c->contentTransferEncoding(true)->decoded())
          dwsrc=c->b_ody.data();
        else {
          dwsrc=c->b_ody.data();
          dwuu.SetAsciiChars(dwsrc);
          dwuu.Decode();
          dwsrc=dwuu.BinaryChars();
        }

        DwEncodeBase64(dwsrc, dwdest);
        c->b_ody=dwdest.c_str();
        c->contentTransferEncoding(true)->setCte(KNHeaders::CEbase64);
        c->contentTransferEncoding(true)->setDecoded(false);
        c->removeHeader("Content-Description");
        c->assemble();
      }
    }

    // add proper mime headers...
    if (convertFromUunec) {
      h_ead.replace(QRegExp("MIME-Version: .*\\n"),"");
      h_ead.replace(QRegExp("Content-Type: .*\\n"),"");
      h_ead.replace(QRegExp("Content-Transfer-Encoding: .*\\n"),"");
      h_ead+="MIME-Version: 1.0\n";
      h_ead+=contentType(true)->as7BitString()+"\n";
      h_ead+=contentTransferEncoding(true)->as7BitString()+"\n";
    }
  }

  //head
  e=h_ead.copy();
  e+="\n";

  //body
  if(!b_ody.isEmpty()) { //this message contains only one part
    KNHeaders::CTEncoding *enc=contentTransferEncoding();

    if(enc->needToEncode()) {
      DwString dwsrc, dwdest;
      dwsrc=b_ody.data();
      if(enc->cte()==KNHeaders::CEquPr)
        DwEncodeQuotedPrintable(dwsrc, dwdest);
      else
        DwEncodeBase64(dwsrc, dwdest);

      e+=dwdest.c_str();
    }
    else
      e+=b_ody;
  }
  else if(c_ontents && !c_ontents->isEmpty()) { //this is a multipart message
    KNHeaders::ContentType *ct=contentType();
    QCString boundary="--"+ct->boundary();

    //add all (encoded) contents separated by boundaries
    for(KNMimeContent *c=c_ontents->first(); c; c=c_ontents->next()) {
      e+=boundary+"\n";
      e+=c->encodedContent(useCrLf);
    }
    //finally append the closing boundary
    e+=boundary+"--\n";
  };

  if(useCrLf)
    return LFtoCRLF(e);
  else
    return e;
}


QByteArray KNMimeContent::decodedContent()
{
  QByteArray ret;
  KNHeaders::CTEncoding *ec=contentTransferEncoding();
  if(ec->decoded())
    ret=b_ody;
  else {
    DwString dwsrc, dwdest;
    dwsrc=b_ody.data();
    DwUuencode dwuu;
    switch(ec->cte()) {
      case KNHeaders::CEbase64 :
        DwDecodeBase64(dwsrc, dwdest);
      break;
      case KNHeaders::CEquPr :
        DwDecodeQuotedPrintable(dwsrc, dwdest);
      break;
      case KNHeaders::CEuuenc :
        dwuu.SetAsciiChars(dwsrc);
        dwuu.Decode();
        dwdest=dwuu.BinaryChars();
      break;
      default :
        dwdest=dwsrc;
    }
    ret.duplicate(dwdest.data(), dwdest.size());
  }

  return ret;
}


void KNMimeContent::decodedText(QString &s, bool trimText)
{
  if(!decodeText()) //this is not a text content !!
    return;

  bool ok=true;
  QTextCodec *codec=KGlobal::charsets()->codecForName(contentType()->charset(),ok);

  s=codec->toUnicode(b_ody.data(), b_ody.length());

  if (trimText && knGlobals.cfgManager->readNewsViewer()->removeTrailingNewlines()) {
    int i;
    for (i=s.length()-1; i>=0; i--)
      if (!s[i].isSpace())
        break;
    s.truncate(i+1);
  }
}


void KNMimeContent::decodedText(QStringList &l, bool trimText)
{
  if(!decodeText()) //this is not a text content !!
    return;

  QString unicode;
  bool ok=true;

  QTextCodec *codec=KGlobal::charsets()->codecForName(contentType()->charset(),ok);

  unicode=codec->toUnicode(b_ody.data(), b_ody.length());

  if (trimText && knGlobals.cfgManager->readNewsViewer()->removeTrailingNewlines()) {
    int i;
    for (i=unicode.length()-1; i>=0; i--)
      if (!unicode[i].isSpace())
        break;
    unicode.truncate(i+1);
  }

  l=QStringList::split('\n', unicode, true); //split the string at linebreaks
}


void KNMimeContent::fromUnicodeString(const QString &s)
{
  bool ok=true;
  QTextCodec *codec=KGlobal::charsets()->codecForName(contentType()->charset(),ok);

  if(!ok) { // no suitable codec found => try local settings and hope the best ;-)
    codec=KGlobal::charsets()->codecForName(KGlobal::locale()->charset(),ok);
    QCString chset=knGlobals.cfgManager->postNewsTechnical()->findComposerCharset(KGlobal::locale()->charset().latin1());
    if (chset.isEmpty())
      chset=KGlobal::locale()->charset().latin1();
    contentType()->setCharset(chset);
  }

  b_ody=codec->fromUnicode(s);
  contentTransferEncoding()->setDecoded(true); //text is always decoded
}


KNMimeContent* KNMimeContent::textContent()
{
  KNMimeContent *ret=0;

  //return the first content with mimetype=text/*
  if(contentType()->isText())
    ret=this;
  else if(c_ontents)
    for(KNMimeContent *c=c_ontents->first(); c; c=c_ontents->next())
      if( (ret=c->textContent())!=0 )
        break;

  return ret;
}


void KNMimeContent::attachments(KNMimeContent::List *dst, bool incAlternatives)
{
  dst->setAutoDelete(false); //don't delete the contents

  if(!c_ontents)
    dst->append(this);
  else {
    for(KNMimeContent *c=c_ontents->first(); c; c=c_ontents->next()) {
      if( !incAlternatives && c->contentType()->category()==KNHeaders::CCalternativePart)
        continue;
      else
        c->attachments(dst, incAlternatives);
    }
  }

  if(type()!=ATmimeContent) { // this is the toplevel article
    KNMimeContent *text=textContent();
    if(text)
      dst->removeRef(text);
  }
}


void KNMimeContent::addContent(KNMimeContent *c, bool prepend)
{
  if(!c_ontents) { // this message is not multipart yet
    c_ontents=new List();
    c_ontents->setAutoDelete(true);

    // first we convert the body to a content
    KNMimeContent *main=new KNMimeContent();

    //the Mime-Headers are needed, so we move them to the new content
    if(h_eaders) {

      main->h_eaders=new KNHeaders::List();
      main->h_eaders->setAutoDelete(true);

      KNHeaders::List srcHdrs=(*h_eaders);
      srcHdrs.setAutoDelete(false);
      int idx=0;
      for(KNHeaders::Base *h=srcHdrs.first(); h; h=srcHdrs.next()) {
        if(h->isMimeHeader()) {
          //remove from this content
          idx=h_eaders->findRef(h);
          h_eaders->take(idx);
          //append to new content
          main->h_eaders->append(h);
        }
      }
    }

    //"main" is now part of a multipart/mixed message
    main->contentType()->setCategory(KNHeaders::CCmixedPart);

    //the head of "main" is empty, so we assemble it
    main->assemble();

    //now we can copy the body and append the new content;
    main->b_ody=b_ody.copy();
    c_ontents->append(main);
    b_ody.resize(0); //not longer needed


    //finally we have to convert this article to "multipart/mixed"
    KNHeaders::ContentType *ct=contentType();
    ct->setMimeType("multipart/mixed");
    ct->setBoundary(multiPartBoundary());
    ct->setCategory(KNHeaders::CCcontainer);
    contentTransferEncoding()->clear();  // 7Bit, decoded

  }
  //here we actually add the content
  if(prepend)
    c_ontents->insert(0, c);
  else
    c_ontents->append(c);
}


void KNMimeContent::removeContent(KNMimeContent *c, bool del)
{
  if(!c_ontents) // what the ..
    return;

  int idx=0;
  if(del)
    c_ontents->removeRef(c);
  else {
    idx=c_ontents->findRef(c);
    c_ontents->take(idx);
  }

  //only one content left => turn this message in a single-part
  if(c_ontents->count()==1) {
    KNMimeContent *main=c_ontents->first();

    //first we have to move the mime-headers
    if(main->h_eaders) {
      if(!h_eaders) {
        h_eaders=new KNHeaders::List();
        h_eaders->setAutoDelete(true);
      }

      KNHeaders::List mainHdrs=(*(main->h_eaders));
      mainHdrs.setAutoDelete(false);

      for(KNHeaders::Base *h=mainHdrs.first(); h; h=mainHdrs.next()) {
        if(h->isMimeHeader()) {
          removeHeader(h->type()); //remove the old header first
          h_eaders->append(h); //now append the new one
          idx=main->h_eaders->findRef(h);
          main->h_eaders->take(idx); //remove from the old content
          kdDebug(5003) << "KNMimeContent::removeContent(KNMimeContent *c, bool del) : mime-header moved: "
                        << h->as7BitString() << endl;
        }
      }
    }

    //now we can copy the body
    b_ody=main->b_ody.copy();

    //finally we can delete the content list
    delete c_ontents;
    c_ontents=0;
  }
}


void KNMimeContent::changeEncoding(KNHeaders::contentEncoding e)
{
  KNHeaders::CTEncoding *enc=contentTransferEncoding();
  if(enc->cte()==e) //nothing to do
    return;

  if(decodeText())
    enc->setCte(e); // text is not encoded until it's sent or saved so we just set the new encoding
  else { // this content contains non textual data, that has to be re-encoded

    if(e!=KNHeaders::CEbase64) {
      //kdWarning(5003) << "KNMimeContent::changeEncoding() : non textual data and encoding != base64 - this should not happen\n => forcing base64" << endl;
      e=KNHeaders::CEbase64;
    }

    if(enc->cte()!=e) { // ok, we reencode the content using base64
      DwString dwsrc, dwdest;
      QByteArray d=decodedContent(); //decode content
      dwsrc.assign(d.data(), d.size());
      DwEncodeBase64(dwsrc, dwdest); //encode as base64
      b_ody=dwdest.c_str(); //set body
      enc->setCte(e); //set encoding
      enc->setDecoded(false);
    }
  }
}


void KNMimeContent::toStream(QTextStream &ts, bool scrambleFromLines)
{
  QCString ret=encodedContent(false);

  if (scrambleFromLines)
    ret.replace(QRegExp("\\n\\nFrom "), "\n\n>From ");

  ts << ret;
}


KNHeaders::Base* KNMimeContent::getHeaderByType(const char *type)
{
  if(!type)
    return 0;

  KNHeaders::Base *h=0;
  //first we check if the requested header is already cached
  if(h_eaders)
    for(h=h_eaders->first(); h; h=h_eaders->next())
      if(h->is(type)) return h; //found

  //now we look for it in the article head
  QCString raw=rawHeader(type);
  if(!raw.isEmpty()) { //ok, we found it
    //choose a suitable header class
    if(strcasecmp("Message-Id", type)==0)
      h=new KNHeaders::MessageID(this, raw);
    else if(strcasecmp("Subject", type)==0)
      h=new KNHeaders::Subject(this, raw);
    else if(strcasecmp("Date", type)==0)
      h=new KNHeaders::Date(this, raw);
    else if(strcasecmp("From", type)==0)
      h=new KNHeaders::From(this, raw);
    else if(strcasecmp("Organization", type)==0)
      h=new KNHeaders::Organization(this, raw);
    else if(strcasecmp("Reply-To", type)==0)
      h=new KNHeaders::ReplyTo(this, raw);
    else if(strcasecmp("Mail-Copies-To", type)==0)
      h=new KNHeaders::MailCopiesTo(this, raw);
    else if(strcasecmp("To", type)==0)
      h=new KNHeaders::To(this, raw);
    else if(strcasecmp("CC", type)==0)
      h=new KNHeaders::CC(this, raw);
    else if(strcasecmp("BCC", type)==0)
      h=new KNHeaders::BCC(this, raw);
    else if(strcasecmp("Newsgroups", type)==0)
      h=new KNHeaders::Newsgroups(this, raw);
    else if(strcasecmp("Followup-To", type)==0)
      h=new KNHeaders::FollowUpTo(this, raw);
    else if(strcasecmp("References", type)==0)
      h=new KNHeaders::References(this, raw);
    else if(strcasecmp("Lines", type)==0)
      h=new KNHeaders::Lines(this, raw);
    else if(strcasecmp("Content-Type", type)==0)
      h=new KNHeaders::ContentType(this, raw);
    else if(strcasecmp("Content-Transfer-Encoding", type)==0)
      h=new KNHeaders::CTEncoding(this, raw);
    else if(strcasecmp("Content-Disposition", type)==0)
      h=new KNHeaders::CDisposition(this, raw);
    else if(strcasecmp("Content-Description", type)==0)
      h=new KNHeaders::CDescription(this, raw);
    else
      h=new KNHeaders::Generic(type, this, raw);

    if(!h_eaders) {
      h_eaders=new KNHeaders::List();
      h_eaders->setAutoDelete(true);
    }

    h_eaders->append(h);  //add to cache
    return h;
  }
  else
    return 0; //header not found
}


void KNMimeContent::setHeader(KNHeaders::Base *h)
{
  if(!h) return;
  removeHeader(h->type());
  if(!h_eaders) {
    h_eaders=new KNHeaders::List();
    h_eaders->setAutoDelete(true);
  }
  h_eaders->append(h);
}


bool KNMimeContent::removeHeader(const char *type)
{
  if(h_eaders)
    for(KNHeaders::Base *h=h_eaders->first(); h; h=h_eaders->next())
      if(h->is(type))
        return h_eaders->remove();

  return false;
}


int KNMimeContent::size()
{
  int ret=b_ody.length();

  if(contentTransferEncoding()->cte()==KNHeaders::CEbase64)
    return (ret*3/4); //base64 => 6 bit per byte

  return ret;
}


int KNMimeContent::storageSize()
{
  int s=h_ead.length();

  if(!c_ontents)
    s+=b_ody.length();
  else {
    for(KNMimeContent *c=c_ontents->first(); c; c=c_ontents->next())
      s+=c->storageSize();
  }

  return s;
}


int KNMimeContent::lineCount()
{
  int ret=0;
  if(type()==ATmimeContent)
    ret+=h_ead.contains('\n');
  ret+=b_ody.contains('\n');

  if(c_ontents && !c_ontents->isEmpty())
    for(KNMimeContent *c=c_ontents->first(); c; c=c_ontents->next())
      ret+=c->lineCount();

  return ret;
}


QCString KNMimeContent::rawHeader(const char *name)
{
  return extractHeader(h_ead, name);
}


bool KNMimeContent::decodeText()
{
  KNHeaders::CTEncoding *enc=contentTransferEncoding();
  if(enc->decoded())
    return true; //nothing to do
  if(!contentType()->isText())
    return false; //non textual data cannot be decoded here => use decodedContent() instead

  DwString dwsrc=b_ody.data(), dwdest;

  if(enc->cte()==KNHeaders::CEquPr)
    DwDecodeQuotedPrintable(dwsrc, dwdest);
  else {
    if (enc->cte()==KNHeaders::CEbase64)
      DwDecodeBase64(dwsrc, dwdest);
    else
      dwdest = dwsrc;           // something has gone wrong...
  }

  b_ody=dwdest.c_str();

  enc->setDecoded(true);
  return true;
}


void KNMimeContent::setForceDefaultCS(bool b)
{
  f_orceDefaultCS=b;
  if (h_eaders)
    h_eaders->clear();
  parse();
}


//==========================================================================================


KNArticle::KNArticle(KNArticleCollection *c) : i_d(-1), c_ol(c), i_tem(0)
{
  s_ubject.setParent(this);
  d_ate.setParent(this);
  l_ines.setParent(this);
}


KNArticle::~KNArticle()
{
  delete i_tem;
}


void KNArticle::parse()
{
  KNMimeContent::parse();

  QCString raw;
  if(s_ubject.isEmpty()) {
    raw=rawHeader(s_ubject.type());
    if(!raw.isEmpty())
      s_ubject.from7BitString(raw);
  }

  if(d_ate.isEmpty()) {
    raw=rawHeader(d_ate.type());
    if(!raw.isEmpty())
      d_ate.from7BitString(raw);
  }

  if(l_ines.isEmpty()) {
    raw=rawHeader(l_ines.type());
    if(!raw.isEmpty())
      l_ines.from7BitString(raw);
  }
}


void KNArticle::assemble()
{
  KNHeaders::Base *h;
  QCString newHead="";

  //Message-ID
  if( (h=messageID(false))!=0 )
    newHead+=h->as7BitString()+"\n";

  //Control
  if( (h=control(false))!=0 )
    newHead+=h->as7BitString()+"\n";

  //Supersedes
  if( (h=supersedes(false))!=0 )
    newHead+=h->as7BitString()+"\n";

  //From
  h=from(); // "From" is mandatory
  newHead+=h->as7BitString()+"\n";

  //Subject
  h=subject(); // "Subject" is mandatory
  newHead+=h->as7BitString()+"\n";

  //To
  if( (h=to(false))!=0 )
    newHead+=h->as7BitString()+"\n";

  //Newsgroups
  if( (h=newsgroups(false))!=0 )
    newHead+=h->as7BitString()+"\n";

  //Followup-To
  if( (h=followUpTo(false))!=0 )
    newHead+=h->as7BitString()+"\n";

  //Reply-To
  if( (h=replyTo(false))!=0 )
    newHead+=h->as7BitString()+"\n";

  //Mail-Copies-To
  if( (h=mailCopiesTo(false))!=0 )
    newHead+=h->as7BitString()+"\n";

  //Date
  h=date(); // "Date" is mandatory
  newHead+=h->as7BitString()+"\n";

  //References
  if( (h=references(false))!=0 )
    newHead+=h->as7BitString()+"\n";

  //Lines
  h=lines(); // "Lines" is mandatory
  newHead+=h->as7BitString()+"\n";

  //Organization
  if( (h=organization(false))!=0 )
    newHead+=h->as7BitString()+"\n";

  //User-Agent
  if( (h=userAgent(false))!=0 )
    newHead+=h->as7BitString()+"\n";

  //Mime-Version
  newHead+="MIME-Version: 1.0\n";

  //Content-Type
  newHead+=contentType()->as7BitString()+"\n";

  //Content-Transfer-Encoding
  newHead+=contentTransferEncoding()->as7BitString()+"\n";

  //X-Headers
  int pos=h_ead.find("\nX-");
  if(pos>-1) //we already have some x-headers => "recycle" them
    newHead+=h_ead.mid(++pos, h_ead.length()-pos);
  else if(h_eaders && !h_eaders->isEmpty()) {
    for(h=h_eaders->first(); h; h=h_eaders->next()) {
      if( h->isXHeader() && (strncasecmp(h->type(), "X-KNode", 7)!=0) )
        newHead+=h->as7BitString()+"\n";
    }
  }

  h_ead=newHead;
}


void KNArticle::clear()
{
  s_ubject.clear();
  d_ate.clear();
  l_ines.clear();
  f_lags.clear();
  KNMimeContent::clear();
}


KNHeaders::Base* KNArticle::getHeaderByType(const char *type)
{
  if(strcasecmp("Subject", type)==0) {
    if(s_ubject.isEmpty()) return 0;
    else return &s_ubject;
  }
  else if(strcasecmp("Date", type)==0){
    if(d_ate.isEmpty()) return 0;
    else return &d_ate;
  }
  else if(strcasecmp("Lines", type)==0) {
    if(l_ines.isEmpty()) return 0;
    else return &l_ines;
  }
  else
    return KNMimeContent::getHeaderByType(type);
}


void KNArticle::setHeader(KNHeaders::Base *h)
{
  bool del=true;
  if(h->is("Subject"))
    s_ubject.fromUnicodeString(h->asUnicodeString(), h->rfc2047Charset());
  else if(h->is("Date"))    
    d_ate.setUnixTime( (static_cast<KNHeaders::Date*>(h))->unixTime() );
  else if(h->is("Lines"))
    l_ines.setNumberOfLines( (static_cast<KNHeaders::Lines*>(h))->numberOfLines() );
  else {
    del=false;
    KNMimeContent::setHeader(h);
  }

  if(del) delete h;
}


bool KNArticle::removeHeader(const char *type)
{
  if(strcasecmp("Subject", type)==0)
    s_ubject.clear();
  else if(strcasecmp("Date", type)==0)    
    d_ate.clear();
  else if(strcasecmp("Lines", type)==0)
    l_ines.clear();
  else
    return KNMimeContent::removeHeader(type);

  return true;
}


void KNArticle::setListItem(KNHdrViewItem *it)
{
  i_tem=it;
  if(i_tem) i_tem->art=this;
}


void KNArticle::setLocked(bool b)
{
  f_lags.set(0, b);
  if(c_ol) {  // local articles may have c_ol==0 !
    if(b)
      c_ol->articleLocked();
    else
      c_ol->articleUnlocked();
  }
}


void KNArticle::setForceDefaultCS(bool b)
{
  s_ubject.clear();
  d_ate.clear();
  l_ines.clear();
  KNMimeContent::setForceDefaultCS(b);
}


//=========================================================================================


KNRemoteArticle::KNRemoteArticle(KNGroup *g)
 : KNArticle(g), a_rticleNumber(-1), i_dRef(-1), d_ref(0), t_hrLevel(0), s_core(0),
   u_nreadFups(0), n_ewFups(0)
{
  m_essageID.setParent(this);
  f_rom.setParent(this);
  r_eferences.setParent(this);

  if (g->useCharset())
    d_efaultCS = cachedCharset( g->defaultCharset() );
  else
    d_efaultCS = cachedCharset( knGlobals.cfgManager->postNewsTechnical()->charset() );
}


KNRemoteArticle::~KNRemoteArticle()
{}


void KNRemoteArticle::parse()
{
  KNArticle::parse();
  QCString raw;
  if(m_essageID.isEmpty() && !(raw=rawHeader(m_essageID.type())).isEmpty() )
    m_essageID.from7BitString(raw);

  if(f_rom.isEmpty() && !(raw=rawHeader(f_rom.type())).isEmpty() )
    f_rom.from7BitString(raw);

  if(r_eferences.isEmpty() && !(raw=rawHeader(r_eferences.type())).isEmpty() )
    r_eferences.from7BitString(raw);
}


void KNRemoteArticle::clear()
{
  m_essageID.clear();
  f_rom.clear();
  r_eferences.clear();
  KNArticle::clear();
}


KNHeaders::Base* KNRemoteArticle::getHeaderByType(const char *type)
{
  if(strcasecmp("Message-ID", type)==0) {
    if(m_essageID.isEmpty()) return 0;
    else return &m_essageID;
  }
  else if(strcasecmp("From", type)==0) {
    if(f_rom.isEmpty()) return 0;
    else return &f_rom;
  }
  else if(strcasecmp("References", type)==0) {
    if(r_eferences.isEmpty()) return 0;
    else return &r_eferences;
  }
  else
    return KNArticle::getHeaderByType(type);
}


void KNRemoteArticle::setHeader(KNHeaders::Base *h)
{
  bool del=true;
  if(h->is("Message-ID"))
    m_essageID.from7BitString(h->as7BitString(false));
  else if(h->is("From")) {    
    f_rom.setEmail( (static_cast<KNHeaders::From*>(h))->email() );
    f_rom.setName( (static_cast<KNHeaders::From*>(h))->name() );
  }
  else if(h->is("References")) {
    r_eferences.from7BitString(h->as7BitString(false));
  }
  else {
    del=false;
    KNArticle::setHeader(h);
  }

  if(del) delete h;
}


bool KNRemoteArticle::removeHeader(const char *type)
{
  if(strcasecmp("Message-ID", type)==0)
    m_essageID.clear();
  else if(strcasecmp("From", type)==0)    
    f_rom.clear();
  else if(strcasecmp("References", type)==0)
    r_eferences.clear();
  else
     return KNArticle::removeHeader(type);

  return true;
}


void KNRemoteArticle::initListItem()
{
  if(!i_tem) return;
  i_tem->setText(0, s_ubject.asUnicodeString());

  if(f_rom.hasName())
    i_tem->setText(1, f_rom.name());
  else
    i_tem->setText(1, QString(f_rom.email()));

  i_tem->setText(3, QString("%1").arg(lines()->numberOfLines()));

  i_tem->setText(4, KGlobal::locale()->formatDateTime(d_ate.qdt(), true));
  updateListItem();
}


void KNRemoteArticle::updateListItem()
{
  if(!i_tem) return;

  KNConfig::Appearance *app=knGlobals.cfgManager->appearance();

  if(isRead()) {
    if(hasContent())
      i_tem->setPixmap(0, app->icon(KNConfig::Appearance::greyBallChkd));
    else
      i_tem->setPixmap(0, app->icon(KNConfig::Appearance::greyBall));
  }
  else {
    if(hasContent())
      i_tem->setPixmap(0,app->icon(KNConfig::Appearance::redBallChkd));
    else
      i_tem->setPixmap(0, app->icon(KNConfig::Appearance::redBall));
  }

  if(hasNewFollowUps())
    i_tem->setPixmap(1, app->icon(KNConfig::Appearance::newFups));
  else
    i_tem->setPixmap(1, app->icon(KNConfig::Appearance::null));

  if(isWatched())
    i_tem->setPixmap(2, app->icon(KNConfig::Appearance::eyes));
  else
    i_tem->setPixmap(2, app->icon(KNConfig::Appearance::null));

  i_tem->setText(2, QString("%1").arg(s_core));
  
  i_tem->setExpandable( (threadMode() && hasVisibleFollowUps()) );

  i_tem->repaint(); //force repaint
}


void KNRemoteArticle::thread(KNRemoteArticle::List &l)
{
  KNRemoteArticle *tmp=0, *ref=this;
  KNGroup *g=static_cast<KNGroup*>(c_ol);
  int idRef=i_dRef, topID=-1;

  while(idRef!=0) {
    ref=g->byId(idRef);
    if(!ref)
      return; // sh#t !!
    idRef=ref->idRef();
  }

  topID=ref->id();
  l.append(ref);

  for(int i=0; i<g->length(); i++) {
    tmp=g->at(i);
    if(tmp->idRef()!=0) {
      idRef=tmp->idRef();
      while(idRef!=0) {
        ref=g->byId(idRef);
        idRef=ref->idRef();
      }
      if(ref->id()==topID)
        l.append(tmp);
    }
  }
}


void KNRemoteArticle::setForceDefaultCS(bool b)
{
  if (!b) { // restore default
    KNGroup *g=static_cast<KNGroup*>(c_ol);
    if (g->useCharset())
      d_efaultCS = cachedCharset( g->defaultCharset() );
    else
      d_efaultCS = cachedCharset( knGlobals.cfgManager->postNewsTechnical()->charset() );
  }
  m_essageID.clear();
  f_rom.clear();
  r_eferences.clear();
  KNArticle::setForceDefaultCS(b);
  initListItem();
}


//=========================================================================================


KNLocalArticle::KNLocalArticle(KNArticleCollection *c)
  : KNArticle(c), s_Offset(-1), e_Offset(-1), s_erverId(-1)
{
  n_ewsgroups.setParent(this);
  t_o.setParent(this);
  d_efaultCS = cachedCharset( knGlobals.cfgManager->postNewsTechnical()->charset() );
}


KNLocalArticle::~KNLocalArticle()
{}


void KNLocalArticle::parse()
{
  KNArticle::parse();
  QCString raw;

  if(n_ewsgroups.isEmpty() && !(raw=rawHeader(n_ewsgroups.type())).isEmpty() )
    n_ewsgroups.from7BitString(raw);

  if(t_o.isEmpty() && !(raw=rawHeader(t_o.type())).isEmpty() )
    t_o.from7BitString(raw);
}


void KNLocalArticle::clear()
{
  KNArticle::clear();
  n_ewsgroups.clear();
  t_o.clear();
}


KNHeaders::Base* KNLocalArticle::getHeaderByType(const char *type)
{
  if(strcasecmp("Newsgroups", type)==0)
    return newsgroups(false);
  else if(strcasecmp("To", type)==0)
    return to(false);
  else
    return KNArticle::getHeaderByType(type);
}


void KNLocalArticle::setHeader(KNHeaders::Base *h)
{
  bool del=true;
  if(h->is("To"))
    t_o.from7BitString(h->as7BitString(false));
  else if(h->is("Newsgroups"))    
    n_ewsgroups.from7BitString(h->as7BitString(false));
  else {
    del=false;
    KNArticle::setHeader(h);
  }

  if(del) delete h;
}


bool KNLocalArticle::removeHeader(const char *type)
{
  if(strcasecmp("To", type)==0)
    t_o.clear();
  else if(strcasecmp("Newsgroups", type)==0)    
    n_ewsgroups.clear();
  else
     return KNArticle::removeHeader(type);

  return true;
}


void KNLocalArticle::updateListItem()
{
  if(!i_tem)
    return;

  i_tem->setText(0, s_ubject.asUnicodeString());

  QString tmp;
  int idx=0;
  KNConfig::Appearance *app=knGlobals.cfgManager->appearance();

  if(isSavedRemoteArticle()) {
    i_tem->setPixmap(0, app->icon(KNConfig::Appearance::savedRemote));
    if (!n_ewsgroups.isEmpty())
      tmp=n_ewsgroups.asUnicodeString();
    else
      tmp=t_o.asUnicodeString();
  }
  else {

    if(doPost()) {
      tmp+=n_ewsgroups.asUnicodeString();
      if(canceled())
        i_tem->setPixmap(idx++, app->icon(KNConfig::Appearance::canceledPosting));
      else
        i_tem->setPixmap(idx++, app->icon(KNConfig::Appearance::posting));
    }

    if(doMail()) {
      i_tem->setPixmap(idx++, app->icon(KNConfig::Appearance::mail));
      if(doPost())
        tmp+=" / ";
      tmp+=t_o.asUnicodeString();
    }

  }

  i_tem->setText(1, tmp);
  i_tem->setText(2, QString::null);
  if (lines()->numberOfLines()!=-1)
    i_tem->setText(3, QString("%1").arg(lines()->numberOfLines()));
  else
    i_tem->setText(3, QString::null);
  i_tem->setText(4, KGlobal::locale()->formatDateTime(d_ate.qdt(), true));
}


void KNLocalArticle::setForceDefaultCS(bool b)
{
  if (!b)  // restore default
    d_efaultCS = cachedCharset( knGlobals.cfgManager->postNewsTechnical()->charset() );
  n_ewsgroups.clear();
  t_o.clear();
  KNArticle::setForceDefaultCS(b);
  updateListItem();
}


//=========================================================================================


KNAttachment::KNAttachment(KNMimeContent *c)
  : c_ontent(c), l_oadHelper(0), f_ile(0), i_sAttached(true)
{
  KNHeaders::ContentType  *t=c->contentType();
  KNHeaders::CTEncoding   *e=c->contentTransferEncoding();
  KNHeaders::CDescription *d=c->contentDescription(false);

  n_ame=t->name();

  if(d)
    d_escription=d->asUnicodeString();


  setMimeType(t->mimeType());

  if(e->cte()==KNHeaders::CEuuenc) {
    setCte( KNHeaders::CEbase64 );
    updateContentInfo();
  }
  else
    e_ncoding.setCte( e->cte() );


  h_asChanged=false; // has been set to "true" in setMimeType()
}


KNAttachment::KNAttachment(KNLoadHelper *helper)
  : c_ontent(0), l_oadHelper(helper), f_ile(helper->getFile()), i_sAttached(false), h_asChanged(true)
{
  setMimeType((KMimeMagic::self()->findFileType(f_ile->name()))->mimeType());
  n_ame=helper->getURL().fileName();
}


KNAttachment::~KNAttachment()
{
  if(!i_sAttached && c_ontent)
    delete c_ontent;
  delete l_oadHelper;
}


void KNAttachment::setMimeType(const QString &s)
{
  m_imeType=s.latin1();
  h_asChanged=true;

  if(m_imeType.find("text/", 0, false)==-1) {
    f_b64=true;
    e_ncoding.setCte(KNHeaders::CEbase64);
  }
  else {
    f_b64=false;
    if (knGlobals.cfgManager->postNewsTechnical()->allow8BitBody())
      setCte(KNHeaders::CE8Bit);
    else
      setCte(KNHeaders::CEquPr);
  }
}


QString KNAttachment::contentSize()
{
  QString ret;
  int s=0;

  if(c_ontent && c_ontent->hasContent())
    s=c_ontent->size();
  else {
    if (f_ile)
      s=f_ile->size();
  }

  if(s > 1023) {
    s=s/1024;
    ret.setNum(s);
    ret+=" kB";
  }
  else {
    ret.setNum(s);
    ret+=" Bytes";
  }

  return ret;
}


void KNAttachment::updateContentInfo()
{
  if(!h_asChanged || !c_ontent)
    return;

  //Content-Type
  KNHeaders::ContentType *t=c_ontent->contentType();
  t->setMimeType(m_imeType);
  t->setName(n_ame, KGlobal::charsets()->charsetForLocale());
  t->setCategory(KNHeaders::CCmixedPart);

  //Content-Description
  if(d_escription.isEmpty())
    c_ontent->removeHeader("Content-Description");
  else
    c_ontent->contentDescription()->fromUnicodeString(d_escription, KGlobal::charsets()->charsetForLocale());

  //Content-Disposition
  KNHeaders::CDisposition *d=c_ontent->contentDisposition();
  d->setDisposition(KNHeaders::CDattachment);
  d->setFilename(n_ame);

  //Content-Transfer-Encoding
  if(i_sAttached)
    c_ontent->changeEncoding(e_ncoding.cte());
  else
    c_ontent->contentTransferEncoding()->setCte(e_ncoding.cte());

  c_ontent->assemble();

  h_asChanged=false;
}



void KNAttachment::attach(KNMimeContent *c)
{
  if(i_sAttached || !f_ile)
    return;

  c_ontent=new KNMimeContent();
  updateContentInfo();
  KNHeaders::ContentType *type=c_ontent->contentType();
  KNHeaders::CTEncoding *e=c_ontent->contentTransferEncoding();

  if(e_ncoding.cte()==KNHeaders::CEbase64 || !type->isText()) { //encode base64

    char *buff=new char[5710];
    int readBytes=0;
    DwString dest;
    DwString src;
    QCString data( (f_ile->size()*4/3)+10 );
    data.at(0)='\0';

    while(!f_ile->atEnd()) {
      // read 5700 bytes at once :
      // 76 chars per line * 6 bit per char / 8 bit per byte => 57 bytes per line
      // we append 100 lines in a row => encode 5700 bytes
      readBytes=f_ile->readBlock(buff, 5700);
      if(readBytes<5700 && f_ile->status()!=IO_Ok) {
        KNHelper::displayExternalFileError();
        delete c_ontent;
        c_ontent=0;
        break;
      }

      src.assign(buff, readBytes);
      DwEncodeBase64(src, dest);
      data+=dest.c_str();
    }

    delete[] buff;

    c_ontent->b_ody=data;
    e->setCte(KNHeaders::CEbase64);
    e->setDecoded(false);
  }
  else { //do not encode text
    QCString txt(f_ile->size()+10);
    int readBytes=f_ile->readBlock(txt.data(), f_ile->size());

    if(readBytes<(int)f_ile->size() && f_ile->status()!=IO_Ok) {
      KNHelper::displayExternalFileError();
      delete c_ontent;
      c_ontent=0;
    }
    else {
      txt[readBytes]='\0'; //terminate string
      c_ontent->b_ody=txt+'\n';
      c_ontent->contentTransferEncoding()->setDecoded(true);
    }
  }

  if(c_ontent) {
    c->addContent(c_ontent);
    i_sAttached=true;
  }
}


void KNAttachment::detach(KNMimeContent *c)
{
  if(i_sAttached) {
    c->removeContent(c_ontent, false);
    i_sAttached=false;
  }
}



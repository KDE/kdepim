/*
    kmime_parsers.cpp

    KMime, the KDE internet mail/usenet news message library.
    Copyright (c) 2001 the KMime authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/
#include "kmime_parsers.h"

#include <qregexp.h>

using namespace KMime::Parser;

namespace KMime {
namespace Parser {


MultiPart::MultiPart(const QCString &src, const QCString &boundary)
{
  s_rc=src;
  b_oundary=boundary;
}


bool MultiPart::parse()
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


NonMimeParser::NonMimeParser(const QCString &src) :
  s_rc(src), p_artNr(-1), t_otalNr(-1)
{}

/**
 * try to guess the mimetype from the file-extension
 */
QCString NonMimeParser::guessMimeType(const QCString& fileName)
{
  QCString tmp, mimeType;
  int pos;

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
    else mimeType="application/octet-stream";
  }
  else mimeType="application/octet-stream";

  return mimeType;
}

//============================================================================================


UUEncoded::UUEncoded(const QCString &src, const QCString &subject) :
  NonMimeParser(src), s_ubject(subject)
{}


bool UUEncoded::parse()
{
  int currentPos=0;
  bool success=true, firstIteration=true;

  while (success) {
    int beginPos=currentPos, uuStart=currentPos, endPos=0, lineCount=0, MCount=0, pos=0, len=0;
    bool containsBegin=false, containsEnd=false;
    QCString tmp,fileName;

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
	QRegExp rx("[0-9]+/[0-9]+");
	pos=rx.search(QString(s_ubject), 0);
	len=rx.matchedLength();
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
      b_ins.append(s_rc.mid(uuStart, endPos-uuStart+1)); //everything beetween "begin" and "end" is uuencoded     
      m_imeTypes.append(guessMimeType(fileName));
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


YENCEncoded::YENCEncoded(const QCString &src) :
  NonMimeParser(src)
{}


bool YENCEncoded::yencMeta(QCString& src, const QCString& name, int* value)
{
  bool found = false;
  QCString sought=name + "=";

  int iPos=src.find( sought);
  if (iPos>-1) {
    int pos1=src.find(' ', iPos);
    int pos2=src.find('\r', iPos);
    int pos3=src.find('\t', iPos);
    int pos4=src.find('\n', iPos);
    if (pos2>=0 && (pos1<0 || pos1>pos2))
      pos1=pos2;
    if (pos3>=0 && (pos1<0 || pos1>pos3))
      pos1=pos3;
    if (pos4>=0 && (pos1<0 || pos1>pos4))
      pos1=pos4;
    iPos=src.findRev( '=', pos1)+1;
    if (iPos<pos1) {
      char c=src.at( iPos);
      if ( c>='0' && c<='9') {
        found=true;
        *value=src.mid( iPos, pos1-iPos).toInt();
      }
    }
  }
  return found;
}


bool YENCEncoded::parse()
{
  int currentPos=0;
  bool success=true;

  while (success) {
    int beginPos=currentPos, yencStart=currentPos;
    bool containsPart=false;
    QCString fileName,mimeType;

    if ((beginPos=s_rc.find("=ybegin ", currentPos))>-1 && ( beginPos==0 || s_rc.at( beginPos-1)=='\n') ) {
      yencStart=s_rc.find( '\n', beginPos);
      if (yencStart==-1) { // no more line breaks found, give up
        success = false;
        break;
      } else {
        yencStart++;
        if (s_rc.find("=ypart", yencStart)==yencStart) {
          containsPart=true;
          yencStart=s_rc.find( '\n', yencStart);
          if ( yencStart== -1) {
            success=false;
            break;
          }
          yencStart++;
        }
      }
      // Try to identify yenc meta data

      // Filenames can contain any embedded chars until end of line
      QCString meta=s_rc.mid(beginPos, yencStart-beginPos);
      int namePos=meta.find("name=");
      if (namePos== -1) {
        success=false;
        break;
      }
      int eolPos=meta.find('\r', namePos);
      if (eolPos== -1)
      eolPos=meta.find('\n', namePos);    
      if (eolPos== -1) {
        success=false;
        break;
      }
      fileName=meta.mid(namePos+5, eolPos-(namePos+5));

      // Other metadata is integer
      int yencLine;
      if (!yencMeta(meta, "line", &yencLine)) {
        success=false;
        break;
      }
      int yencSize;
      if (!yencMeta( meta, "size", &yencSize)) {
        success=false;
        break;
      }

      int partBegin, partEnd;
      if (containsPart) {
        if (!yencMeta(meta, "part", &p_artNr)) {
          success=false;
          break;
        }
        if (!yencMeta(meta, "begin", &partBegin) || !
             yencMeta(meta, "end", &partEnd)) {
          success=false;
          break;
        }
        if (!yencMeta(meta, "total", &t_otalNr))
          t_otalNr=p_artNr+1;
        if (yencSize==partEnd-partBegin+1)
          t_otalNr=1; else
        yencSize=partEnd-partBegin+1;
      }

      // We have a valid yenc header; now we extract the binary data
      int totalSize=0;
      int pos=yencStart;
      int len=s_rc.length();
      bool lineStart=true;
      int lineLength=0;
      bool containsEnd=false;
      QByteArray binary = QByteArray(yencSize);
      while (pos<len) {
        int ch=s_rc.at(pos);
        if (ch<0)
          ch+=256;
        if (ch=='\r')
        {
          if (lineLength!=yencLine && totalSize!=yencSize)          
            break;          
          pos++;
        }
        else if (ch=='\n')
        {
          lineStart=true;
          lineLength=0;
          pos++;
        }
        else
        {
          if (ch=='=')
          {
            if (pos+1<len)
            {
              ch=s_rc.at( pos+1);
              if (lineStart && ch=='y')
              {
                containsEnd=true;
                break;
              }
              pos+=2;
              ch-=64+42;
              if (ch<0)
                ch+=256;
              if (totalSize>=yencSize)            
                break;            
              binary.at(totalSize++)=ch;
              lineLength++;
            }
            else            
              break;            
          }
          else
          {
            ch-=42;
            if (ch<0)
              ch+=256;
            if (totalSize>=yencSize)            
              break;
            binary.at(totalSize++)=ch;
            lineLength++;
            pos++;
          }
          lineStart=false;
        }
      }
      
      if (!containsEnd)
      {
        success=false;
        break;
      }
      if (totalSize!=yencSize)
      {        
        success=false;
        break;
      }

      // pos now points to =yend; get end data
      eolPos=s_rc.find('\n', pos);
      if (eolPos== -1)
      {
        success=false;
        break;
      }
      meta=s_rc.mid(pos, eolPos-pos);
      if (!yencMeta(meta, "size", &totalSize))
      {        
        success=false;
        break;
      }
      if (totalSize!=yencSize)
      {        
        success=false;
        break;
      }

      f_ilenames.append(fileName);
      m_imeTypes.append(guessMimeType( fileName));
      b_ins.append(binary);

      //everything before "begin" is text
      if(beginPos>0)
        t_ext.append(s_rc.mid(currentPos,beginPos-currentPos));
      currentPos = eolPos+1;

    } else {
      success = false;
    }
  }

  // append trailing text part of the article
  t_ext.append(s_rc.right(s_rc.length()-currentPos));

  return b_ins.count()>0;
}

} // namespace Parser
} // namespace KMime

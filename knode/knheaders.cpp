/*
    knheaders.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2000 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include <qtextcodec.h>
#include <qstringlist.h>
#include <mimelib/datetime.h>

#include <kdebug.h>

#include "knheaders.h"
#include "knmime.h"
#include "knstringsplitter.h"


KNHeaders::Generic::Generic(const char *t) : t_ype(0)
{
  setType(t);
}


KNHeaders::Generic::Generic(const char *t, const QCString &s): t_ype(0)
{
  setType(t);
  from7BitString(s);
}


KNHeaders::Generic::Generic(const char *t, const QString &s, QFont::CharSet cs) : t_ype(0)
{
  setType(t);
  fromUnicodeString(s, cs);
}


KNHeaders::Generic::~Generic()
{
  delete[] t_ype;
}


void KNHeaders::Generic::from7BitString(const QCString &s)
{
  u_nicode=KNMimeBase::decodeRFC2047String(s, e_ncCSet);
}


QCString KNHeaders::Generic::as7BitString(bool incType)
{
  if(incType)
    return ( typeIntro()+KNMimeBase::encodeRFC2047String(u_nicode, e_ncCSet) );
  else
    return KNMimeBase::encodeRFC2047String(u_nicode, e_ncCSet);
}


void KNHeaders::Generic::fromUnicodeString(const QString &s, QFont::CharSet cs)
{
  u_nicode=s;
  e_ncCSet=cs;
}


QString KNHeaders::Generic::asUnicodeString()
{
  return u_nicode;
}


void KNHeaders::Generic::setType(const char *type)
{
  if(t_ype)
    delete[] t_ype;
  if(type) {
    t_ype=new char[strlen(type)+1];
    strcpy(t_ype, type);
  }
  else
    t_ype=0;
}


//==============================================================================================


void KNHeaders::MessageID::from7BitString(const QCString &s)
{
  m_id=s;
  e_ncCSet=QFont::ISO_8859_1;
}



QCString KNHeaders::MessageID::as7BitString(bool incType)
{
  if(incType)
    return ( typeIntro()+m_id );
  else
    return m_id;
}


void KNHeaders::MessageID::fromUnicodeString(const QString &s, QFont::CharSet)
{
  m_id=s.latin1(); //Message-Ids can only contain us-ascii chars
  e_ncCSet=QFont::ISO_8859_1;
}



QString KNHeaders::MessageID::asUnicodeString()
{
  return QString::fromLatin1(m_id);
}


void KNHeaders::MessageID::generate(const QCString &fqdn)
{
  m_id="<"+KNMimeBase::uniqueString()+"@"+fqdn+">";
}


//==============================================================================================


void KNHeaders::Control::from7BitString(const QCString &s)
{
  c_trlMsg=s;
  e_ncCSet=QFont::ISO_8859_1;
}


QCString KNHeaders::Control::as7BitString(bool incType)
{
  if(incType)
    return ( typeIntro()+c_trlMsg );
  else
    return c_trlMsg;
}


void KNHeaders::Control::fromUnicodeString(const QString &s, QFont::CharSet)
{
  c_trlMsg=s.latin1();
  e_ncCSet=QFont::ISO_8859_1;
}


QString KNHeaders::Control::asUnicodeString()
{
  return QString::fromLatin1(c_trlMsg);
}


//==============================================================================================


void KNHeaders::Subject::from7BitString(const QCString &s)
{
  s_ubject=KNMimeBase::decodeRFC2047String(s, e_ncCSet);
}


QCString KNHeaders::Subject::as7BitString(bool incType)
{
  if(incType)
    return (typeIntro()+KNMimeBase::encodeRFC2047String(s_ubject, e_ncCSet));
  else
    return KNMimeBase::encodeRFC2047String(s_ubject, e_ncCSet);
}


void KNHeaders::Subject::fromUnicodeString(const QString &s, QFont::CharSet cs)
{
  s_ubject=s;
  e_ncCSet=cs;
}


QString KNHeaders::Subject::asUnicodeString()
{
  return s_ubject;
}


//==============================================================================================


void KNHeaders::AddressField::from7BitString(const QCString &s)
{
  int pos1=0, pos2=0, type=0;
  QCString n;

  //so what do we have here ?
  if(s.find( QRegExp("*@*(*)", false, true) )!=-1) type=2;       // From: foo@bar.com (John Doe)
  else if(s.find( QRegExp("*<*@*>", false, true) )!=-1) type=1;  // From: John Doe <foo@bar.com>
  else if(s.find( QRegExp("*@*", false, true) )!=-1) type=0;     // From: foo@bar.com
  else { //broken From header => just decode it
    n_ame=KNMimeBase::decodeRFC2047String(s, e_ncCSet);
    return;
  }

  switch(type) {

    case 0:
      e_mail=s.copy();
    break;

    case 1:
      pos1=0;
      pos2=s.find('<');
      if(pos2!=-1) {
        n=s.mid(pos1, pos2-pos1).stripWhiteSpace();
        pos1=pos2+1;
        pos2=s.find('>', pos1);
        if(pos2!=-1)
          e_mail=s.mid(pos1, pos2-pos1);
      }
      else return;
    break;

    case 2:
      pos1=0;
      pos2=s.find('(');
      if(pos2!=-1) {
        e_mail=s.mid(pos1, pos2-pos1).stripWhiteSpace();
        pos1=pos2+1;
        pos2=s.find(')', pos1);
        if(pos2!=-1)
          n=s.mid(pos1, pos2-pos1).stripWhiteSpace();
      }
    break;

    default: break;
  }

  if(!n.isEmpty()) {
    KNMimeBase::removeQuots(n);
    n_ame=KNMimeBase::decodeRFC2047String(n, e_ncCSet);
  }
  else
    e_ncCSet=QFont::ISO_8859_1;
}


QCString KNHeaders::AddressField::as7BitString(bool incType)
{
  QCString ret;

  if(incType && type()[0]!='\0')
    ret=typeIntro();

  if(n_ame.isEmpty())
    ret+=e_mail;
  else
    ret+=KNMimeBase::encodeRFC2047String(n_ame, e_ncCSet)+" <"+e_mail+">";

  return ret;
}


void KNHeaders::AddressField::fromUnicodeString(const QString &s, QFont::CharSet cs)
{
  int pos1=0, pos2=0, type=0;
  QCString n;
  e_ncCSet=cs;

  //so what do we have here ?
  if(s.find( QRegExp("*@*(*)", false, true) )!=-1) type=2;       // From: foo@bar.com (John Doe)
  else if(s.find( QRegExp("*<*@*>", false, true) )!=-1) type=1;  // From: John Doe <foo@bar.com>
  else if(s.find( QRegExp("*@*", false, true) )!=-1) type=0;     // From: foo@bar.com
  else { //broken From header => just copy it
    n_ame=s.copy();
    return;
  }

  switch(type) {

    case 0:
      e_mail=s.latin1();
    break;

    case 1:
      pos1=0;
      pos2=s.find('<');
      if(pos2!=-1) {
        n_ame=s.mid(pos1, pos2-pos1).stripWhiteSpace();
        pos1=pos2+1;
        pos2=s.find('>', pos1);
        if(pos2!=-1)
          e_mail=s.mid(pos1, pos2-pos1).latin1();
      }
      else return;
    break;

    case 2:
      pos1=0;
      pos2=s.find('(');
      if(pos2!=-1) {
        e_mail=s.mid(pos1, pos2-pos1).stripWhiteSpace().latin1();
        pos1=pos2+1;
        pos2=s.find(')', pos1);
        if(pos2!=-1)
          n_ame=s.mid(pos1, pos2-pos1).stripWhiteSpace();
      }
    break;

    default: break;
  }

  if(!n_ame.isEmpty())
    KNMimeBase::removeQuots(n_ame);
}


QString KNHeaders::AddressField::asUnicodeString()
{
  if(n_ame.isEmpty())
    return QString(e_mail);
  else
    return QString("%1 <%2>").arg(n_ame).arg(QString::fromLatin1(e_mail));
}


//==============================================================================================


void KNHeaders::Organization::from7BitString(const QCString &s)
{
  o_rga=KNMimeBase::decodeRFC2047String(s, e_ncCSet);
}


QCString KNHeaders::Organization::as7BitString(bool incType)
{
  if(incType)
    return ( typeIntro()+KNMimeBase::encodeRFC2047String(o_rga, e_ncCSet) );
  else
    return KNMimeBase::encodeRFC2047String(o_rga, e_ncCSet);
}


void KNHeaders::Organization::fromUnicodeString(const QString &s, QFont::CharSet cs)
{
  o_rga=s;
  e_ncCSet=cs;
}


QString KNHeaders::Organization::asUnicodeString()
{
  return o_rga;
}


//==============================================================================================


void KNHeaders::Date::from7BitString(const QCString &s)
{
  DwDateTime dt;
  dt.FromString(s.data());
  dt.Parse();
  t_ime=dt.AsUnixTime();
  e_ncCSet=QFont::ISO_8859_1;
}


QCString KNHeaders::Date::as7BitString(bool incType)
{
  DwDateTime dt;
  dt.FromUnixTime(t_ime);
  dt.Assemble();

  if(incType)
    return ( typeIntro()+dt.AsString().c_str() );
  else
    return QCString(dt.AsString().c_str());
}


void KNHeaders::Date::fromUnicodeString(const QString &s, QFont::CharSet)
{
  from7BitString( QCString(s.latin1()) );
}


QString KNHeaders::Date::asUnicodeString()
{
  return QString::fromLatin1(as7BitString(false));
}


QDateTime KNHeaders::Date::qdt()
{
  QDateTime dt;
  dt.setTime_t(t_ime);
  return dt;
}


int KNHeaders::Date::ageInDays()
{
	QDate today=QDate::currentDate();
	return ( qdt().date().daysTo(today) );
}


//==============================================================================================


void KNHeaders::To::from7BitString(const QCString &s)
{
  if(a_ddrList)
    a_ddrList->clear();
  else
    a_ddrList=new QValueList<AddressField>;

  KNStringSplitter split;
  split.init(s, ",");
  bool splitOk=split.first();
  if(!splitOk)
    *a_ddrList << AddressField(s);
  else
    while(splitOk)
      *a_ddrList << AddressField(split.string());

  e_ncCSet=a_ddrList->first().rfc2047Charset();
}


QCString KNHeaders::To::as7BitString(bool incType)
{
  QCString ret;

  if(incType)
    ret+=typeIntro();

  AddressList::Iterator it=a_ddrList->begin();
  ret+=(*it).as7BitString(false);
  ++it;
  for(; it!=a_ddrList->end(); ++it)
    ret+=","+(*it).as7BitString(false);

  return ret;
}


void KNHeaders::To::fromUnicodeString(const QString &s, QFont::CharSet cs)
{
  if(a_ddrList)
    a_ddrList->clear();
  else
    a_ddrList=new QValueList<AddressField>;

  QStringList l=QStringList::split(",", s);

  QStringList::Iterator it=l.begin();
  for(; it!=l.end(); ++it)
    *a_ddrList << AddressField( (*it), cs );

  e_ncCSet=cs;
}


QString KNHeaders::To::asUnicodeString()
{
  if(!a_ddrList)
    return QString::null;

  QString ret;
  AddressList::Iterator it=a_ddrList->begin();
  ret+=(*it).asUnicodeString();
  ++it;

  for(; it!=a_ddrList->end(); ++it)
    ret+=","+(*it).asUnicodeString();

  return ret;
}


void KNHeaders::To::addAddress(const AddressField &a)
{
  if(!a_ddrList)
    a_ddrList=new QValueList<AddressField>;

  a_ddrList->append(a);
}


void KNHeaders::To::emails(QStrList *l)
{
  l->clear();
  AddressList::Iterator it=a_ddrList->begin();
  for(; it!=a_ddrList->end(); ++it)
    if( (*it).hasEmail() )
      l->append( (*it).email() );
}


//==============================================================================================


void KNHeaders::Newsgroups::from7BitString(const QCString &s)
{
  g_roups=s;
  e_ncCSet=QFont::ISO_8859_1;
}


QCString KNHeaders::Newsgroups::as7BitString(bool incType)
{
  if(incType)
    return (typeIntro()+g_roups);
  else
    return g_roups;
}


void KNHeaders::Newsgroups::fromUnicodeString(const QString &s, QFont::CharSet)
{
  g_roups=s.latin1();
  e_ncCSet=QFont::ISO_8859_1;
}


QString KNHeaders::Newsgroups::asUnicodeString()
{
  return QString::fromLatin1(g_roups);
}


QCString KNHeaders::Newsgroups::firstGroup()
{
  int pos=0;
  if(!g_roups.isEmpty()) {
    pos=g_roups.find(',');
    if(pos==-1)
      return g_roups;
    else
      return g_roups.left(pos);
  }
  else
    return QCString();
}


//==============================================================================================


void KNHeaders::Lines::from7BitString(const QCString &s)
{
  l_ines=s.toInt();
  e_ncCSet=QFont::ISO_8859_1;
}


QCString KNHeaders::Lines::as7BitString(bool incType)
{
  QCString num;
  num.setNum(l_ines);

  if(incType)
    return ( typeIntro()+num );
  else
    return num;
}


void KNHeaders::Lines::fromUnicodeString(const QString &s, QFont::CharSet)
{
  l_ines=s.toInt();
  e_ncCSet=QFont::ISO_8859_1;
}


QString KNHeaders::Lines::asUnicodeString()
{
  QString num;
  num.setNum(l_ines);

  return num;
}


//==============================================================================================


void KNHeaders::References::from7BitString(const QCString &s)
{
  r_ef=s;
  e_ncCSet=QFont::ISO_8859_1;
}


QCString KNHeaders::References::as7BitString(bool incType)
{
  if(incType)
    return ( typeIntro()+r_ef );
  else
    return r_ef;
}


void KNHeaders::References::fromUnicodeString(const QString &s, QFont::CharSet)
{
  r_ef=s.latin1();
  e_ncCSet=QFont::ISO_8859_1;
}


QString KNHeaders::References::asUnicodeString()
{
  return QString::fromLatin1(r_ef);
}


int KNHeaders::References::count()
{
  int cnt1=0, cnt2=0;
  unsigned int r_efLen=r_ef.length();
  char *dataPtr=r_ef.data();
  for(unsigned int i=0; i<r_efLen; i++) {
    if(dataPtr[i]=='<') cnt1++;
    else if(dataPtr[i]=='>') cnt2++;
  }

  if(cnt1<cnt2) return cnt1;
  else return cnt2;
}


QCString KNHeaders::References::first()
{
  pos=-1;
  return next();
}


QCString KNHeaders::References::next()
{
  int pos1, pos2;
  QCString ret;

  if(pos!=0) {
    pos2=r_ef.findRev('>', pos);
    pos=0;
    if(pos2!=-1) {
      pos1=r_ef.findRev('<', pos2);
      if(pos1!=-1) {
        ret=r_ef.mid(pos1, pos2-pos1+1);
        pos=pos1;
      }
    }
  }
  return ret;
}


QCString KNHeaders::References::at(unsigned int i)
{
  QCString ret;
  int pos1=0, pos2=0;
  unsigned int cnt=0;

  while(pos1!=-1 && cnt < i+1) {
    pos2=pos1-1;
    pos1=r_ef.findRev('<', pos2);
    cnt++;
  }

  if(pos1!=-1) {
    pos2=r_ef.find('>', pos1);
    if(pos2!=-1)
      ret=r_ef.mid(pos1, pos2-pos1+1);
  }

 return ret;
}


//==============================================================================================


void KNHeaders::UserAgent::from7BitString(const QCString &s)
{
  u_agent=s;
  e_ncCSet=QFont::ISO_8859_1;
}


QCString KNHeaders::UserAgent::as7BitString(bool incType)
{
  if(incType)
    return ( typeIntro()+u_agent );
  else
    return u_agent;
}


void KNHeaders::UserAgent::fromUnicodeString(const QString &s, QFont::CharSet)
{
  u_agent=s.latin1();
  e_ncCSet=QFont::ISO_8859_1;
}


QString KNHeaders::UserAgent::asUnicodeString()
{
  return QString::fromLatin1(u_agent);
}


//==============================================================================================


void KNHeaders::ContentType::from7BitString(const QCString &s)
{
  kdDebug(5003) << "KNHeaders::ContentType::from7BitString(const QCString &s) : s = " << s << endl;
  int pos=s.find(';');

  if(pos==-1)
    m_imeType=s;
  else {
    m_imeType=s.left(pos);
    p_arams=s.mid(pos, s.length()-pos);
  }

  if(isMultipart())
    c_ategory=CCcontainer;
  else
    c_ategory=CCsingle;

  e_ncCSet=QFont::ISO_8859_1;
}


QCString KNHeaders::ContentType::as7BitString(bool incType)
{
  if(incType)
    return (typeIntro()+m_imeType+p_arams);
  else
    return (m_imeType+p_arams);
}


void KNHeaders::ContentType::fromUnicodeString(const QString &s, QFont::CharSet)
{
  from7BitString(QCString(s.latin1()));
  e_ncCSet=QFont::ISO_8859_1;
}


QString KNHeaders::ContentType::asUnicodeString()
{
  return QString::fromLatin1(as7BitString(false));
}


QCString KNHeaders::ContentType::mediaType()
{
  int pos=m_imeType.find('/');
  if(pos==-1)
    return m_imeType;
  else
    return m_imeType.left(pos);
}


QCString KNHeaders::ContentType::subType()
{
  int pos=m_imeType.find('/');
  if(pos==-1)
    return QCString();
  else
    return m_imeType.mid(pos, m_imeType.length()-pos);
}


void KNHeaders::ContentType::setMimeType(const QCString &s)
{
  p_arams.resize(0);
  m_imeType=s;

  if(isMultipart())
    c_ategory=CCcontainer;
  else
    c_ategory=CCsingle;
}


bool KNHeaders::ContentType::isMediatype(const char *s)
{
  return ( strncasecmp(m_imeType.data(), s, strlen(s)) );
}


bool KNHeaders::ContentType::isSubtype(const char *s)
{
  char *c=strchr(m_imeType.data(), '/')+1;

  if( (c==0) || (*c)=='\0' )
    return false;
  else
    return ( strcasecmp(c, s)==0 );
}


bool KNHeaders::ContentType::isText()
{
  return (strncasecmp(m_imeType.data(), "text", 4)==0);
}


bool KNHeaders::ContentType::isPlainText()
{
  return (strcasecmp(m_imeType.data(), "text/plain")==0);
}


bool KNHeaders::ContentType::isHTMLText()
{
  return (strcasecmp(m_imeType.data(), "text/html")==0);
}


bool KNHeaders::ContentType::isImage()
{
  return (strncasecmp(m_imeType.data(), "image", 5)==0);
}


bool KNHeaders::ContentType::isMultipart()
{
  return (strncasecmp(m_imeType.data(), "multipart", 9)==0);
}


bool KNHeaders::ContentType::isPartial()
{
  return (strcasecmp(m_imeType.data(), "message/partial")==0);
}


QCString KNHeaders::ContentType::charset()
{
  QCString ret=getParameter("charset").upper();
  if(ret.isEmpty()) // || ret=="US-ASCII")
    ret="ISO-8859-1";
  return ret;
}


void KNHeaders::ContentType::setCharset(const QCString &s)
{
  setParameter("charset", s.upper());
}


QCString KNHeaders::ContentType::boundary()
{
  return getParameter("boundary");
}


void KNHeaders::ContentType::setBoundary(const QCString &s)
{
  setParameter("boundary", s, true);
}


QString KNHeaders::ContentType::name()
{
  QFont::CharSet dummy;
  return ( KNMimeBase::decodeRFC2047String(getParameter("name"), dummy) );
}


void KNHeaders::ContentType::setName(const QCString &s)
{
  setParameter("name", s, true);
}


QCString KNHeaders::ContentType::id()
{
  return (getParameter("id"));
}


void KNHeaders::ContentType::setId(const QCString &s)
{
  setParameter("id", s, true);
}


int KNHeaders::ContentType::partialNumber()
{
  QCString p=getParameter("number");
  if(!p.isEmpty())
    return p.toInt();
  else
    return -1;
}


int KNHeaders::ContentType::partialCount()
{
  QCString p=getParameter("total");
  if(!p.isEmpty())
    return p.toInt();
  else
    return -1;
}


void KNHeaders::ContentType::setPartialParams(int total, int number)
{
  QCString num;
  num.setNum(number);
  setParameter("number", num);
  num.setNum(total);
  setParameter("total", num);
}


QCString KNHeaders::ContentType::getParameter(const char *name)
{
  QCString ret;
  int pos1=0, pos2=0;
  pos1=p_arams.find(name, 0, false);
  if(pos1!=-1) {
    if( (pos2=p_arams.find(';', pos1))==-1 )
      pos2=p_arams.length();
    pos1+=strlen(name)+1;
    ret=p_arams.mid(pos1, pos2-pos1);
    KNMimeBase::removeQuots(ret);
    //kdDebug(5003) << "KNHeaders::ContentType::getParameter() : " << name << " = " << ret << endl;
  }
  return ret;
}


void KNHeaders::ContentType::setParameter(const QCString &name, const QCString &value, bool doubleQuotes)
{
  int pos1=0, pos2=0;
  QCString param;

  if(doubleQuotes)
    param=name+"=\""+value+"\"";
  else
    param=name+"="+value;

  pos1=p_arams.find(name, 0, false);
  if(pos1==-1) {
    p_arams+="; "+param;
  }
  else {
    pos2=p_arams.find(';', pos1);
    if(pos2==-1)
      pos2=p_arams.length();
    p_arams.remove(pos1, pos2-pos1);
    p_arams.insert(pos1, param);
  }
}


//==============================================================================================


typedef struct { const char *s; int e; } encTableType;

static const encTableType encTable[] = {  { "7Bit", KNHeaders::CE7Bit },
                                          { "8Bit", KNHeaders::CE8Bit },
                                          { "quoted-printable", KNHeaders::CEquPr },
                                          { "base64", KNHeaders::CEbase64 },
                                          { "x-uuencode", KNHeaders::CEuuenc },
                                          { "binary", KNHeaders::CEbinary },
                                          { 0, 0} };


void KNHeaders::CTEncoding::from7BitString(const QCString &s)
{
  c_te=CE7Bit;
  for(int i=0; encTable[i].s!=0; i++)
    if(strcasecmp(s.data(), encTable[i].s)==0) {
      c_te=(contentEncoding)encTable[i].e;
      break;
    }
  d_ecoded=( c_te==CE7Bit || c_te==CE8Bit );

  e_ncCSet=QFont::ISO_8859_1;
}


QCString KNHeaders::CTEncoding::as7BitString(bool incType)
{
  QCString str;
  for(int i=0; encTable[i].s!=0; i++)
    if(c_te==encTable[i].e) {
      str=encTable[i].s;
      break;
    }

  if(incType)
    return ( typeIntro()+str );
  else
    return str;
}


void KNHeaders::CTEncoding::fromUnicodeString(const QString &s, QFont::CharSet)
{
  from7BitString(QCString(s.latin1()));
}


QString KNHeaders::CTEncoding::asUnicodeString()
{
  return QString::fromLatin1(as7BitString(false));
}


//==============================================================================================


void KNHeaders::CDisposition::from7BitString(const QCString &s)
{
  if(strncasecmp(s.data(), "attachment", 10)==0)
    d_isp=CDattachment;
  else d_isp=CDinline;

  int pos=s.find("filename=", 0, false);
  QCString fn;
  if(pos>-1) {
    pos+=9;
    fn=s.mid(pos, s.length()-pos);
    KNMimeBase::removeQuots(fn);
    f_ilename=KNMimeBase::decodeRFC2047String(fn, e_ncCSet);
  }
}


QCString KNHeaders::CDisposition::as7BitString(bool incType)
{
  QCString ret;
  if(d_isp==CDattachment)
    ret="attachment";
  else
    ret="inline";

  if(!f_ilename.isEmpty())
    ret+="; filename=\""+KNMimeBase::encodeRFC2047String(f_ilename, e_ncCSet)+"\"";

  if(incType)
    return ( typeIntro()+ret );
  else
    return ret;
}


void KNHeaders::CDisposition::fromUnicodeString(const QString &s, QFont::CharSet cs)
{
  if(strncasecmp(s.latin1(), "attachment", 10)==0)
    d_isp=CDattachment;
  else d_isp=CDinline;

  int pos=s.find("filename=", 0, false);
  if(pos>-1) {
    pos+=9;
    f_ilename=s.mid(pos, s.length()-pos);
    KNMimeBase::removeQuots(f_ilename);
  }

  e_ncCSet=cs;
}


QString KNHeaders::CDisposition::asUnicodeString()
{
  QString ret;
  if(d_isp==CDattachment)
    ret="attachment";
  else
    ret="inline";

  if(!f_ilename.isEmpty())
    ret+="; filename=\""+f_ilename+"\"";

  return ret;
}


//==============================================================================================


void KNHeaders::CDescription::from7BitString(const QCString &s)
{
  d_esc=KNMimeBase::decodeRFC2047String(s, e_ncCSet);
}


QCString KNHeaders::CDescription::as7BitString(bool incType)
{
  if(incType)
    return ( typeIntro()+KNMimeBase::encodeRFC2047String(d_esc, e_ncCSet) );
  else
    return KNMimeBase::encodeRFC2047String(d_esc, e_ncCSet);
}


void KNHeaders::CDescription::fromUnicodeString(const QString &s, QFont::CharSet cs)
{
  d_esc=s;
  e_ncCSet=cs;
}


QString KNHeaders::CDescription::asUnicodeString()
{
  return d_esc;
}

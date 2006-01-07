/*
    kmime_message.cpp

    KMime, the KDE internet mail/usenet news message library.
    Copyright (c) 2001 the KMime authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include "kmime_message.h"

using namespace KMime;

namespace KMime {

Message::Message()
{
  s_ubject.setParent(this);
  d_ate.setParent(this);
}

Message::~Message() {}

void Message::parse()
{
  Content::parse();

  QByteArray raw;
  if( !(raw=rawHeader(s_ubject.type())).isEmpty() )
    s_ubject.from7BitString(raw);

  if( !(raw=rawHeader(d_ate.type())).isEmpty() )
    d_ate.from7BitString(raw);
}


void Message::assemble()
{
  Headers::Base *h;
  QByteArray newHead="";

  //Message-ID
  if( (h=messageID(false))!=0 )
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

  //Reply-To
  if( (h=replyTo(false))!=0 )
    newHead+=h->as7BitString()+"\n";

  //Date
  h=date(); // "Date" is mandatory
  newHead+=h->as7BitString()+"\n";

  //References
  if( (h=references(false))!=0 )
    newHead+=h->as7BitString()+"\n";

  //Organization
  if( (h=organization(false))!=0 )
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
    newHead+=h_ead.mid(pos+1, h_ead.length()-pos-1);
  else if(h_eaders && !h_eaders->isEmpty()) {
    foreach ( Headers::Base *h, *h_eaders ) {
      if ( h->isXHeader() && ( strncasecmp( h->type(), "X-KNode", 7 ) != 0 ) )
        newHead += h->as7BitString() + '\n';
    }
  }

  h_ead=newHead;
}


void Message::clear()
{
  s_ubject.clear();
  d_ate.clear();
  f_lags.clear();
  Content::clear();
}


Headers::Base* Message::getHeaderByType(const char *type)
{
  if(strcasecmp("Subject", type)==0) {
    if(s_ubject.isEmpty()) return 0;
    else return &s_ubject;
  }
  else if(strcasecmp("Date", type)==0){
    if(d_ate.isEmpty()) return 0;
    else return &d_ate;
  }
  else
    return Content::getHeaderByType(type);
}


void Message::setHeader(Headers::Base *h)
{
  bool del=true;
  if(h->is("Subject"))
    s_ubject.fromUnicodeString(h->asUnicodeString(), h->rfc2047Charset());
  else if(h->is("Date"))
    d_ate.setUnixTime( (static_cast<Headers::Date*>(h))->unixTime() );
  else {
    del=false;
    Content::setHeader(h);
  }

  if(del) delete h;
}


bool Message::removeHeader(const char *type)
{
  if(strcasecmp("Subject", type)==0)
    s_ubject.clear();
  else if(strcasecmp("Date", type)==0)
    d_ate.clear();
  else
    return Content::removeHeader(type);

  return true;
}




} // namespace KMime

/*
    kmime_newsarticle.cpp

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
#include "kmime_newsarticle.h"

using namespace KMime;

namespace KMime {

void NewsArticle::parse()
{
  Message::parse();

  QCString raw;

  if( !(raw=rawHeader(l_ines.type())).isEmpty() )
    l_ines.from7BitString(raw);
}

void NewsArticle::assemble()
{
  Headers::Base *h;
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
    newHead+=h_ead.mid(pos+1, h_ead.length()-pos);
  else if(h_eaders && !h_eaders->isEmpty()) {
    for(h=h_eaders->first(); h; h=h_eaders->next()) {
      if( h->isXHeader() && (strncasecmp(h->type(), "X-KNode", 7)!=0) )
        newHead+=h->as7BitString()+"\n";
    }
  }

  h_ead=newHead;
}

void NewsArticle::clear()
{
  l_ines.clear();
  Message::clear();
}

Headers::Base * NewsArticle::getHeaderByType(const char * type)
{
  if(strcasecmp("Lines", type)==0) {
    if(l_ines.isEmpty()) return 0;
    else return &l_ines;
  } else
    return Message::getHeaderByType(type);
}

void NewsArticle::setHeader(Headers::Base *h)
{
  bool del=true;
  if(h->is("Lines"))
    l_ines.setNumberOfLines( (static_cast<Headers::Lines*>(h))->numberOfLines() );
  else {
    del=false;
    Message::setHeader(h);
  }

  if(del) delete h;
}


bool NewsArticle::removeHeader(const char *type)
{
  if(strcasecmp("Lines", type)==0)
    l_ines.clear();
  else
    return Message::removeHeader(type);

  return true;
}


} // namespace KMime

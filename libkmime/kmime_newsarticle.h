/*
    kmime_newsarticle.h

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
#ifndef __KMIME_NEWSARTICLE_H__
#define __KMIME_NEWSARTICLE_H__

#include "kmime_message.h"

#include <kdemacros.h>

namespace KMime {

class KDE_EXPORT NewsArticle : public Message {

public:

  NewsArticle() : Message() { l_ines.setParent(this); }
  ~NewsArticle() {};

  virtual void parse();
  virtual void assemble();
  virtual void clear();

  virtual KMime::Headers::Base * getHeaderByType(const char* type);
  virtual void setHeader(KMime::Headers::Base *h);
  virtual bool removeHeader(const char * type);

  virtual KMime::Headers::Control* control(bool create=true)            { KMime::Headers::Control *p=0; return getHeaderInstance(p, create); }
  virtual KMime::Headers::Supersedes* supersedes(bool create=true)      { KMime::Headers::Supersedes *p=0; return getHeaderInstance(p, create); }
  virtual KMime::Headers::MailCopiesTo* mailCopiesTo(bool create=true)  { KMime::Headers::MailCopiesTo *p=0; return getHeaderInstance(p, create); }
  virtual KMime::Headers::Newsgroups* newsgroups(bool create=true)      { KMime::Headers::Newsgroups *p=0; return getHeaderInstance(p, create); }
  virtual KMime::Headers::FollowUpTo* followUpTo(bool create=true)      { KMime::Headers::FollowUpTo *p=0; return getHeaderInstance(p, create); }
  virtual KMime::Headers::Lines* lines(bool create=true)                { if(!create && l_ines.isEmpty()) return 0; return &l_ines; }
  virtual KMime::Headers::UserAgent* userAgent(bool create=true)        { KMime::Headers::UserAgent *p=0; return getHeaderInstance(p, create); }


protected:
  KMime::Headers::Lines l_ines;

}; // class NewsArticle

} // namespace KMime

#endif // __KMIME_NEWSARTICLE_H__

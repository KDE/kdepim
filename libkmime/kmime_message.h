/*
    kmime_message.h

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
#ifndef __KMIME_MESSAGE_H__
#define __KMIME_MESSAGE_H__

#include "kmime_content.h"
#include "kmime_headers.h"
#include "boolflags.h"

namespace KMime {

class KDE_EXPORT Message : public Content {

public:
  typedef QPtrList<Message> List;

  /** Constructor. Creates an empty message. */
  Message();
  ~Message();

  //content handling
  virtual void parse();
  virtual void assemble();
  virtual void clear();

  //header access
  virtual KMime::Headers::Base* getHeaderByType(const char *type);
  virtual void setHeader(KMime::Headers::Base *h);
  virtual bool removeHeader(const char *type);

  virtual KMime::Headers::MessageID* messageID(bool create=true)        { KMime::Headers::MessageID *p=0; return getHeaderInstance(p, create); }
  virtual KMime::Headers::Subject* subject(bool create=true)            { if(!create && s_ubject.isEmpty()) return 0; return &s_ubject; }
  virtual KMime::Headers::Date* date(bool create=true)                  { if(!create && d_ate.isEmpty()) return 0;return &d_ate; }
  virtual KMime::Headers::From* from(bool create=true)                  { KMime::Headers::From *p=0; return getHeaderInstance(p, create); }
  virtual KMime::Headers::Organization* organization(bool create=true)  { KMime::Headers::Organization *p=0; return getHeaderInstance(p, create); }
  virtual KMime::Headers::ReplyTo* replyTo(bool create=true)            { KMime::Headers::ReplyTo *p=0; return getHeaderInstance(p, create); }
  virtual KMime::Headers::To* to(bool create=true)                      { KMime::Headers::To *p=0; return getHeaderInstance(p, create); }
  virtual KMime::Headers::CC* cc(bool create=true)                      { KMime::Headers::CC *p=0; return getHeaderInstance(p, create); }
  virtual KMime::Headers::BCC* bcc(bool create=true)                    { KMime::Headers::BCC *p=0; return getHeaderInstance(p, create); }
  virtual KMime::Headers::References* references(bool create=true)      { KMime::Headers::References *p=0; return getHeaderInstance(p, create); }
  
protected:
  //hardcoded headers
  KMime::Headers::Subject s_ubject;
  KMime::Headers::Date d_ate;
  BoolFlags f_lags; // some status info


}; // class Message

} // namespace KMime

#endif // __KMIME_MESSAGE_H__

/*
    kmime_content.h

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
#ifndef __KMIME_CONTENT_H__
#define __KMIME_CONTENT_H__

//forward declarations
#if 0
class KMime::Headers::Base;
class KMime::Headers::Generic;
class KMime::Headers::ContentType;
class KMime::Headers::CTEncoding;
class KMime::Headers::CDisposition;
class KMime::Headers::List;
#endif

#include "kmime_util.h"
#include "kmime_headers.h"

#include <qtextstream.h>

namespace KMime {


/** Base class for messages in mime format
    It contains all the enums, static functions
    and parser-classes, that are needed for
    mime handling */

class Base {

  public:

    //enums
    enum articleType    { ATmimeContent,
                          ATremote,
                          ATlocal };

};


/** This class encapsulates a mime-encoded content.
    It parses the given data and creates a tree-like
    structure, that represents the structure of the
    message */

class Content : public Base {

  public:
    typedef QPtrList<KMime::Content> List;

    Content();
    Content(const QCString &h, const QCString &b);
    virtual ~Content();

    //type
    virtual articleType type()      { return ATmimeContent; }

    //content handling
    bool hasContent()               { return ( !h_ead.isEmpty() && (!b_ody.isEmpty() || (c_ontents && !c_ontents->isEmpty())) ); }
    void setContent(QStrList *l);
    void setContent(const QCString &s);
    virtual void parse();
    virtual void assemble();
    virtual void clear();

    //header access
    QCString head()       { return h_ead; }
    // extracts and removes the next header from head. The caller has to delete the returned header;
    Headers::Generic*  getNextHeader(QCString &head);
    virtual Headers::Base* getHeaderByType(const char *type);
    virtual void setHeader(Headers::Base *h);
    virtual bool removeHeader(const char *type);
    bool hasHeader(const char *type)                                  { return (getHeaderByType(type)!=0); }
    Headers::ContentType* contentType(bool create=true)             { Headers::ContentType *p=0; return getHeaderInstance(p, create); }
    Headers::CTEncoding* contentTransferEncoding(bool create=true)  { Headers::CTEncoding *p=0; return getHeaderInstance(p, create); }
    Headers::CDisposition* contentDisposition(bool create=true)     { Headers::CDisposition *p=0; return getHeaderInstance(p, create); }
    Headers::CDescription* contentDescription(bool create=true)     { Headers::CDescription *p=0; return getHeaderInstance(p, create); }

    //content access
    int size();
    int storageSize();
    int lineCount();
    QCString body()       { return b_ody; }
    void setBody( const QCString & str ) { b_ody = str; }
    QCString encodedContent(bool useCrLf=false);
    QByteArray decodedContent();
    void decodedText(QString &s, bool trimText=false,
		     bool removeTrainingNewlines=false);
    void decodedText(QStringList &s, bool trimText=false,
		     bool removeTrainingNewlines=false);
    void fromUnicodeString(const QString &s);

    Content* textContent();
    void attachments(List *dst, bool incAlternatives=false);
    void addContent(Content *c, bool prepend=false);
    void removeContent(Content *c, bool del=false);
    void changeEncoding(Headers::contentEncoding e);

    //saves the encoded content to the given textstream
    // scrambleFromLines: replace "\nFrom " with "\n>From ", this is
    // needed to avoid problem with mbox-files
    void toStream(QTextStream &ts, bool scrambleFromLines=false);

    // this charset is used for all headers and the body
    // if the charset is not declared explictly
    QCString defaultCharset()                  { return QCString(d_efaultCS); }
    void setDefaultCharset(const QCString &cs);

    // use the default charset even if a different charset is
    // declared in the article
    bool forceDefaultCS()         {  return f_orceDefaultCS; }

    // enables/disables the force mode, housekeeping.
    // works correctly only when the article is completely empty or
    // completely loaded
    virtual void setForceDefaultCS(bool b);


  protected:
    QCString rawHeader(const char *name);
    bool decodeText();
    template <class T> T* getHeaderInstance(T *ptr, bool create);

    QCString  h_ead,
              b_ody;
    List *c_ontents;
    Headers::Base::List *h_eaders;
    const char *d_efaultCS;
    bool f_orceDefaultCS;

};

// some compilers (for instance Compaq C++) need template inline functions
// here rather than in the *.cpp file

template <class T> T* Content::getHeaderInstance(T *ptr, bool create)
{
  T dummy; //needed to access virtual member T::type()

  ptr=static_cast <T*> (getHeaderByType(dummy.type()));
  if(!ptr && create) { //no such header found, but we need one => create it
    ptr=new T(this);
    if(!(h_eaders)) {
      h_eaders=new Headers::Base::List();
      h_eaders->setAutoDelete(true);
    }
    h_eaders->append(ptr);
  }

  return ptr;
}



} // namespace KMime

#endif // __KMIME_CONTENT_H__

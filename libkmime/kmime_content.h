/*
    kmime_content.h

    KMime, the KDE internet mail/usenet news message library.
    Copyright (c) 2001 the KMime authors.
    See file AUTHORS for details
    Copyright (c) 2006 Volker Krause <volker.krause@rwth-aachen.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
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

#include <kmime_contentindex.h>
#include "kmime_util.h"
#include "kmime_headers.h"

#include <QTextStream>
#include <QByteArray>
#include <QList>

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

class ContentPrivate;

/** This class encapsulates a mime-encoded content.
    It parses the given data and creates a tree-like
    structure, that represents the structure of the
    message */

class KDE_EXPORT Content : public Base {

  public:
    typedef QList<KMime::Content*> List;

    /**
      Creates an empty Content object.
    */
    Content();
    /**
      Creates a Content object containing the given raw data.
      @param h The header data.
      @param b The body data.
    */
    Content( const QByteArray &h, const QByteArray &b );
    /**
      Destroys this Content object.
    */
    virtual ~Content();

    //type
    virtual articleType type() const { return ATmimeContent; }

    /**
      Returns true if this Content object is not empty.
    */
    bool hasContent() const;
    /**
      Sets the content to the given raw data, containing the content head and
      body separated by two linefeeds.
      @param l a line-splitted list of the raw content data.
    */
    void setContent( const QList<QByteArray> & l );
    /**
      Sets the content to the given raw data, containing the content head and
      body separated by two linefeeds.
      @param s a QByteArray containing the raw content data.
    */
    void setContent( const QByteArray &s );
    virtual void parse();
    virtual void assemble();
    /**
      Clears the complete message and deletes all sub-contents.
    */
    virtual void clear();

    /**
      Returns the content header raw data.
    */
    QByteArray head() const;
    /**
      Sets the content header raw data.
    */
    void setHead( const QByteArray &head );

    /**
      Extracts and removes the next header from head.
      The caller has to delete the returned header.
    */
    Headers::Generic*  getNextHeader(QByteArray &head);
    virtual Headers::Base* getHeaderByType(const char *type);
    virtual void setHeader(Headers::Base *h);
    virtual bool removeHeader(const char *type);
    bool hasHeader(const char *type)                                  { return (getHeaderByType(type)!=0); }
    /**
      Returns the content type header.
      @param create Create the header if it doesn't exist yet.
    */
    Headers::ContentType* contentType(bool create=true)             { Headers::ContentType *p=0; return getHeaderInstance(p, create); }
    Headers::CTEncoding* contentTransferEncoding(bool create=true)  { Headers::CTEncoding *p=0; return getHeaderInstance(p, create); }
    Headers::CDisposition* contentDisposition(bool create=true)     { Headers::CDisposition *p=0; return getHeaderInstance(p, create); }
    Headers::CDescription* contentDescription(bool create=true)     { Headers::CDescription *p=0; return getHeaderInstance(p, create); }

    /**
      Returns the size of the content body after encoding.
    */
    int size();
    /**
      Returns the size of this content and all sub-contents.
    */
    int storageSize() const;
    /**
      Line count of this content and all sub-contents.
    */
    int lineCount() const;
    /**
      Returns the content body raw data.
    */
    QByteArray body() const;
    /**
      Sets the content body raw data.
    */
    void setBody( const QByteArray &body );
    /**
      Returns a QByteArray containing the encoded content, including the
      content header and all sub-contents.
      @param useCrLf Use CRLF instead of LF for linefeeds.
    */
    QByteArray encodedContent( bool useCrLf = false );
    /**
      Returns the decoded content body.
    */
    QByteArray decodedContent();
    /**
      Returns the decoded text. Additional to decodedContent(), this also
      applies charset decoding. If this is not a text content, decodedText()
      returns an empty QString.
    */
    QString decodedText( bool trimText = false, bool removeTrailingNewlines = false );
    /**
      Sets the content body to the given string using the current charset.
      @param s Unicode-encoded string.
    */
    void fromUnicodeString(const QString &s);

    /**
      Returns the first content with mimetype text/.
    */
    Content* textContent();
    /**
      Returns a list of attachments.
      @param incAlternatives include multipart/alternative parts.
    */
    List attachments( bool incAlternatives = false );
    /**
      Returns a list of sub-contents.
    */
    List contents() const;
    /**
      Adds a new sub-content, the current Content object is converted into a
      multipart/mixed content node if it has been a single-part content.
      @param c The new sub-content.
      @param prepend Prepend to content list instead of append.
    */
    void addContent( Content *c, bool prepend = false );
    /**
      Removes the given sub-content, the current Content object is converted
      into a single-port content if only one sub-content is left.
      @param c The content to remove.
      @param del Delete the removed content object.
    */
    void removeContent( Content *c, bool del = false );
    void changeEncoding(Headers::contentEncoding e);

    /**
      Saves the encoded content to the given textstream
      @param ts The stream where the content should be written to.
      @param scrambleFromLines: replace "\nFrom " with "\n>From ", this is
      needed to avoid problem with mbox-files
    */
    void toStream( QTextStream &ts, bool scrambleFromLines = false );

    /**
      This charset is used for all headers and the body
      if the charset is not declared explictly.
    */
    QByteArray defaultCharset() const;
    /**
      Sets the default charset.
      @param cs The new default charset.
    */
    void setDefaultCharset( const QByteArray &cs );

    /**
      Use the default charset even if a different charset is
      declared in the article.
    */
    bool forceDefaultCharset() const;

    /**
      Enables/disables the force mode, housekeeping.
      works correctly only when the article is completely empty or
      completely loaded.
      @param b True to force usage of the default charset.
    */
    virtual void setForceDefaultCharset( bool b );

    /**
      Returns the content specified by the given index.
      If the index doesn't point to an content, 0 is returned, if the index
      is invalid (empty), this content is returned.
      @param index the content index
    */
    Content* content( const ContentIndex &index ) const;

    /**
      Returns the ContentIndex for the given content, an invalid index
      if the content is not found withing the hierarchy.
      @param content the Content object to search.
    */
    ContentIndex indexForContent( Content *content ) const;


  protected:
    QByteArray rawHeader(const char *name);
    bool decodeText();
    template <class T> T* getHeaderInstance(T *ptr, bool create);

    Headers::Base::List h_eaders;

  private:
    ContentPrivate* const d;
};

// some compilers (for instance Compaq C++) need template inline functions
// here rather than in the *.cpp file

template <class T> T* Content::getHeaderInstance(T *ptr, bool create)
{
  T dummy; //needed to access virtual member T::type()

  ptr=static_cast <T*> (getHeaderByType(dummy.type()));
  if(!ptr && create) { //no such header found, but we need one => create it
    ptr=new T(this);
    h_eaders.append( ptr );
  }

  return ptr;
}



} // namespace KMime

#endif // __KMIME_CONTENT_H__

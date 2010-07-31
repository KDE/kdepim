/**************************************************************************
*   Copyright (C) 2003 - 2004 by Frerich Raabe <raabe@kde.org>            *
*                                Tobias Koenig <tokoe@kde.org>            *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/
#ifndef KXMLRPCIFACE_H
#define KXMLRPCIFACE_H

#include <kurl.h>

#include <tqbuffer.h>
#include <tqdom.h>
#include <tqobject.h>
#include <tqvariant.h>
#include <tqvaluelist.h>

namespace KIO
{
  class Job;
}

namespace KXMLRPC
{
  class Query;
  class QueryResult;
  class Server;
  class Result;

  class Query : public QObject
  {
    Q_OBJECT
    public:


      static Query *create( const TQVariant &id = TQVariant(),
                            TQObject *parent = 0, const char *name = 0 );

    public slots:
      void call( const TQString &server, const TQString &method,
                 const TQValueList<TQVariant> &args = TQValueList<TQVariant>(),
                 const TQString &userAgent = "KDE-XMLRPC" );

    signals:
      void message( const TQValueList<TQVariant> &result, const TQVariant &id );
      void fault( int, const TQString&, const TQVariant &id );
      void finished( Query* );

    private slots:
      void slotData( KIO::Job *job, const TQByteArray &data );
      void slotResult( KIO::Job *job );

    private:
      bool isMessageResponse( const TQDomDocument &doc ) const;
      bool isFaultResponse( const TQDomDocument &doc ) const;

      Result parseMessageResponse( const TQDomDocument &doc ) const;
      Result parseFaultResponse( const TQDomDocument &doc ) const;

      TQString markupCall( const TQString &method,
                          const TQValueList<TQVariant> &args ) const;
      TQString marshal( const TQVariant &v ) const;
      TQVariant demarshal( const TQDomElement &e ) const;

      Query( const TQVariant &id, TQObject *parent = 0, const char *name = 0 );
      ~Query();

      TQByteArray m_buffer;
      TQVariant m_id;

      TQValueList<KIO::Job*> m_pendingJobs;
  };

  class Server : public QObject
  {
    Q_OBJECT
    public:
      Server( const KURL &url = KURL(),
              TQObject *parent = 0, const char *name = 0 );
      ~Server();

      const KURL &url() const { return m_url; }
      void setUrl( const KURL &url );

      TQString userAgent() const { return m_userAgent; }
      void setUserAgent( const TQString &userAgent ) { m_userAgent = userAgent; }

      template <typename T>
      void call( const TQString &method, const TQValueList<T> &arg,
        TQObject* obj1, const char* faultSlot,
        TQObject* obj2, const char* messageSlot, const TQVariant &id = TQVariant() );


    public slots:
      void call( const TQString &method, const TQValueList<TQVariant> &args,
        TQObject* faultObj, const char* faultSlot,
        TQObject* msgObj, const char* messageSlot,
        const TQVariant &id = TQVariant() );
      void call( const TQString &method, const TQVariant &arg,
        TQObject* faultObj, const char* faultSlot,
        TQObject* msgObj, const char* messageSlot,
        const TQVariant &id = TQVariant() );
      void call( const TQString &method, int arg ,
        TQObject* faultObj, const char* faultSlot,
        TQObject* msgObj, const char* messageSlot,
        const TQVariant &id = TQVariant() );
      void call( const TQString &method, bool arg,
        TQObject* faultObj, const char* faultSlot,
        TQObject* msgObj, const char* messageSlot,
        const TQVariant &id = TQVariant() );
      void call( const TQString &method, double arg,
        TQObject* faultObj, const char* faultSlot,
        TQObject* msgObj, const char* messageSlot,
        const TQVariant &id = TQVariant() );
      void call( const TQString &method, const TQString &arg,
        TQObject* faultObj, const char* faultSlot,
        TQObject* msgObj, const char* messageSlot,
        const TQVariant &id = TQVariant() );
      void call( const TQString &method, const TQCString &arg ,
        TQObject* faultObj, const char* faultSlot,
        TQObject* msgObj, const char* messageSlot,
        const TQVariant &id = TQVariant() );
      void call( const TQString &method, const TQByteArray &arg,
        TQObject* faultObj, const char* faultSlot,
        TQObject* msgObj, const char* messageSlot,
        const TQVariant &id = TQVariant() );
      void call( const TQString &method, const TQDateTime &arg,
        TQObject* faultObj, const char* faultSlot,
        TQObject* msgObj, const char* messageSlot,
        const TQVariant &id = TQVariant() );
      void call( const TQString &method, const TQStringList &arg,
        TQObject* faultObj, const char* faultSlot,
        TQObject* msgObj, const char* messageSlot,
        const TQVariant &id = TQVariant() );

    private slots:
      void queryFinished( Query* );

    private:
      KURL m_url;
      TQString m_userAgent;

      TQValueList<Query*> mPendingQueries;
  };
}

template <typename T>
void KXMLRPC::Server::call( const TQString &method, const TQValueList<T> &arg,
        TQObject* faultObj, const char* faultSlot,
        TQObject* msgObj, const char* messageSlot, const TQVariant &id )
{
  TQValueList<TQVariant> args;

  typename TQValueList<T>::ConstIterator it = arg.begin();
  typename TQValueList<T>::ConstIterator end = arg.end();
  for ( ; it != end; ++it )
    args << TQVariant( *it );

  return call( method, args, faultObj, faultSlot, msgObj, messageSlot, id );
}

#endif

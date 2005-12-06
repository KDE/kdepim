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

#include <qbuffer.h>
#include <qdom.h>
#include <qobject.h>
#include <qvariant.h>
#include <q3valuelist.h>
//Added by qt3to4:
#include <Q3CString>

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


      static Query *create( const QVariant &id = QVariant(),
                            QObject *parent = 0, const char *name = 0 );

    public slots:
      void call( const QString &server, const QString &method,
                 const Q3ValueList<QVariant> &args = Q3ValueList<QVariant>(),
                 const QString &userAgent = "KDE-XMLRPC" );

    signals:
      void message( const Q3ValueList<QVariant> &result, const QVariant &id );
      void fault( int, const QString&, const QVariant &id );
      void finished( Query* );

    private slots:
      void slotData( KIO::Job *job, const QByteArray &data );
      void slotResult( KIO::Job *job );

    private:
      bool isMessageResponse( const QDomDocument &doc ) const;
      bool isFaultResponse( const QDomDocument &doc ) const;

      Result parseMessageResponse( const QDomDocument &doc ) const;
      Result parseFaultResponse( const QDomDocument &doc ) const;

      QString markupCall( const QString &method,
                          const Q3ValueList<QVariant> &args ) const;
      QString marshal( const QVariant &v ) const;
      QVariant demarshal( const QDomElement &e ) const;

      Query( const QVariant &id, QObject *parent = 0, const char *name = 0 );
      ~Query();

      QByteArray m_buffer;
      QVariant m_id;

      Q3ValueList<KIO::Job*> m_pendingJobs;
  };

  class Server : public QObject
  {
    Q_OBJECT
    public:
      Server( const KURL &url = KURL(),
              QObject *parent = 0, const char *name = 0 );
      ~Server();

      const KURL &url() const { return m_url; }
      void setUrl( const KURL &url );

      QString userAgent() const { return m_userAgent; }
      void setUserAgent( const QString &userAgent ) { m_userAgent = userAgent; }

      template <typename T>
      void call( const QString &method, const Q3ValueList<T> &arg,
        QObject* obj, const char* faultSlot,
        QObject* obj, const char* messageSlot, const QVariant &id = QVariant() );


    public slots:
      void call( const QString &method, const Q3ValueList<QVariant> &args,
        QObject* faultObj, const char* faultSlot,
        QObject* msgObj, const char* messageSlot,
        const QVariant &id = QVariant() );
      void call( const QString &method, const QVariant &arg,
        QObject* faultObj, const char* faultSlot,
        QObject* msgObj, const char* messageSlot,
        const QVariant &id = QVariant() );
      void call( const QString &method, int arg ,
        QObject* faultObj, const char* faultSlot,
        QObject* msgObj, const char* messageSlot,
        const QVariant &id = QVariant() );
      void call( const QString &method, bool arg,
        QObject* faultObj, const char* faultSlot,
        QObject* msgObj, const char* messageSlot,
        const QVariant &id = QVariant() );
      void call( const QString &method, double arg,
        QObject* faultObj, const char* faultSlot,
        QObject* msgObj, const char* messageSlot,
        const QVariant &id = QVariant() );
      void call( const QString &method, const QString &arg,
        QObject* faultObj, const char* faultSlot,
        QObject* msgObj, const char* messageSlot,
        const QVariant &id = QVariant() );
      void call( const QString &method, const Q3CString &arg ,
        QObject* faultObj, const char* faultSlot,
        QObject* msgObj, const char* messageSlot,
        const QVariant &id = QVariant() );
      void call( const QString &method, const QByteArray &arg,
        QObject* faultObj, const char* faultSlot,
        QObject* msgObj, const char* messageSlot,
        const QVariant &id = QVariant() );
      void call( const QString &method, const QDateTime &arg,
        QObject* faultObj, const char* faultSlot,
        QObject* msgObj, const char* messageSlot,
        const QVariant &id = QVariant() );
      void call( const QString &method, const QStringList &arg,
        QObject* faultObj, const char* faultSlot,
        QObject* msgObj, const char* messageSlot,
        const QVariant &id = QVariant() );

    private slots:
      void queryFinished( Query* );

    private:
      KURL m_url;
      QString m_userAgent;

      Q3ValueList<Query*> mPendingQueries;
  };
}

template <typename T>
void KXMLRPC::Server::call( const QString &method, const Q3ValueList<T> &arg,
        QObject* faultObj, const char* faultSlot,
        QObject* msgObj, const char* messageSlot, const QVariant &id )
{
  Q3ValueList<QVariant> args;

  typename Q3ValueList<T>::ConstIterator it = arg.begin();
  typename Q3ValueList<T>::ConstIterator end = arg.end();
  for ( ; it != end; ++it )
    args << QVariant( *it );

  return call( method, args, faultObj, faultSlot, msgObj, messageSlot, id );
}

#endif

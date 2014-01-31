/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/* This file is part of QWebdav
 *
 * Copyright (C) 2009-2010 Corentin Chary <corentin.chary@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef QWEBDAV_H
#define QWEBDAV_H

#include <QHttp>
#include <QHttpRequestHeader>
#include <QUrlInfo>
#include <QDateTime>
#include <QDomNodeList>

namespace PimCommon {
class QWebdavUrlInfo;

/**
 * @brief Main class used to handle the webdav protocol
 */
class QWebdav : public QHttp
{
    Q_OBJECT
public:
    explicit QWebdav ( QObject * parent = 0 );
    QWebdav ( const QString & hostName, quint16 port = 80, QObject * parent = 0 );
    QWebdav ( const QString & hostName, ConnectionMode mode, quint16 port = 0,
              QObject * parent = 0 );
    ~QWebdav ();

    typedef QMap < QString, QMap < QString, QVariant > > PropValues;
    typedef QMap < QString , QStringList > PropNames;

    int list ( const QString & dir = QString() );
    int search ( const QString & path, const QString & query );
    int put ( const QString & path, QIODevice *data );
    int put ( const QString & path, const QByteArray & data );

    int mkcol ( const QString & dir );

    int mkdir ( const QString & dir );
    int copy ( const QString & oldname, const QString & newname,
               bool overwrite = false );
    int rename ( const QString & oldname, const QString & newname,
                 bool overwrite = false );
    int move ( const QString & oldname, const QString & newname,
               bool overwrite = false );
    int rmdir ( const QString & dir );
    int remove ( const QString & path );

    int propfind ( const QString & path, const QByteArray & query, int depth = 0 );
    int propfind ( const QString & path, const QWebdav::PropNames & props,
                   int depth = 0 );

    int proppatch ( const QString & path, const QWebdav::PropValues & props);
    int proppatch ( const QString & path, const QByteArray & query );

    int setHost ( const QString &, quint16 );
    int setHost ( const QString &, ConnectionMode, quint16 );

    /* TODO lock, unlock */
Q_SIGNALS:
    void listInfo(const QString &data);


private slots:
    void readyRead ( const QHttpResponseHeader & resp );
    void requestFinished ( int id, bool error );
    void responseHeaderReceived( const QHttpResponseHeader & resp );

private:
    void init(const QString & hostName);
    void emitListInfos();
    void davParsePropstats( const QDomNodeList & propstat );
    int codeFromResponse( const QString& response );
    QDateTime parseDateTime( const QString& input, const QString& type );
    int davRequest(QHttpRequestHeader & req,
                   const QByteArray & data = QByteArray());
    int davRequest(QHttpRequestHeader & req,
                   QIODevice * data);
    void setupHeaders(QHttpRequestHeader & req, quint64 size);

private:
    Q_DISABLE_COPY(QWebdav)
    bool emitListInfo;
    QByteArray buffer;
    QString host;
};
}

#endif // QWEBDAV_H

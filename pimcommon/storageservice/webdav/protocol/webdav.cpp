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

#include <QDomDocument>
#include <QDomNode>
#include <QUrl>
#include <QDebug>

#include "webdav.h"
#include "webdav_url_info.h"

using namespace PimCommon;

QWebdav::QWebdav (QObject *parent)
    : QHttp(parent)
{
    init(QString());
}

QWebdav::QWebdav (const QString &hostName, quint16 port, QObject *parent)
    : QHttp(hostName, port, parent)
{
    init(hostName);
}

QWebdav::QWebdav ( const QString & hostName, ConnectionMode mode, quint16 port,
                   QObject * parent)
    : QHttp(hostName, mode, port, parent)
{
    init(hostName);
}

QWebdav::~QWebdav ()
{
}

int QWebdav::setHost ( const QString & hostName, quint16 port )
{
    host = hostName;
    return QHttp::setHost(hostName, port);
}

int QWebdav::setHost ( const QString & hostName, ConnectionMode mode, quint16 port)
{
    host = hostName;
    return QHttp::setHost(hostName, mode, port);
}

void QWebdav::init(const QString & hostName)
{
    host = hostName;
    emitListInfo = false;
    connect(this, SIGNAL(readyRead(QHttpResponseHeader)),
            this, SLOT(readyRead(QHttpResponseHeader)));
    connect(this, SIGNAL(requestFinished(int,bool)),
            this, SLOT(requestFinished(int,bool)));
    connect(this, SIGNAL(responseHeaderReceived(QHttpResponseHeader)),
            this, SLOT(responseHeaderReceived(QHttpResponseHeader)));
}

int QWebdav::list ( const QString & dir)
{
    QWebdav::PropNames query;
    QStringList props;

    props << QLatin1String("creationdate");
    props << QLatin1String("getcontentlength");
    props << QLatin1String("displayname");
    props << QLatin1String("source");
    props << QLatin1String("getcontentlanguage");
    props << QLatin1String("getcontenttype");
    props << QLatin1String("executable");
    props << QLatin1String("getlastmodified");
    props << QLatin1String("getetag");
    props << QLatin1String("resourcetype");

    query[QLatin1String("DAV:")] = props;

    return propfind(dir, query, 1);
}

int QWebdav::search ( const QString & path, const QString & q )
{
    QByteArray query = "<?xml version=\"1.0\"?>\r\n";

    query.append( "<D:searchrequest xmlns:D=\"DAV:\">\r\n" );
    query.append( q.toUtf8() );
    query.append( "</D:searchrequest>\r\n" );

    QHttpRequestHeader req(QLatin1String("SEARCH"), path);
    return davRequest(req);
}

int QWebdav::put ( const QString & path, QIODevice * data )
{
    QHttpRequestHeader req(QLatin1String("PUT"), path);

    return davRequest(req, data);
}

int QWebdav::put ( const QString & path, const QByteArray & data )
{
    QHttpRequestHeader req(QLatin1String("PUT"), path);

    return davRequest(req, data);
}

int QWebdav::propfind ( const QString & path, const QWebdav::PropNames & props, int depth)
{
    QByteArray query;

    query = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>";
    query += "<D:propfind xmlns:D=\"DAV:\" >";
    query += "<D:prop>";
    foreach (const QString &ns, props.keys())
    {
        foreach (const QString &key, props[ns])
            if (ns == QLatin1String("DAV:"))
                query += "<D:" + key.toLatin1() + "/>";
            else
                query += "<" + key.toLatin1() + " xmlns=\"" + ns.toLatin1() + "\"/>";
    }
    query += "</D:prop>";
    query += "</D:propfind>";
    return propfind(path, query, depth);
}


int QWebdav::propfind( const QString & path, const QByteArray & query, int depth )
{
    QHttpRequestHeader req(QLatin1String("PROPFIND"), path);
    QString value;

    if (depth == 2)
        value = QLatin1String("infinity");
    else
        value = QString::fromLatin1("%1").arg(depth);
    req.setValue(QLatin1String("Depth"), value);
    return davRequest(req, query);
}

int QWebdav::proppatch ( const QString & path, const QWebdav::PropValues & props)
{
    QByteArray query;

    query = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>";
    query += "<D:proppatch xmlns:D=\"DAV:\" >";
    query += "<D:prop>";
    foreach (const QString &ns, props.keys())
    {
        QMap < QString , QVariant >::const_iterator i;

        for (i = props[ns].constBegin(); i != props[ns].constEnd(); ++i) {
            if (ns == QLatin1String("DAV:")) {
                query += "<D:" + i.key().toLatin1() + ">";
                query += i.value().toString().toLatin1();
                query += "</D:" + i.key().toLatin1() + ">" ;
            } else {
                query += "<" + i.key().toLatin1() + " xmlns=\"" + ns.toLatin1() + "\">";
                query += i.value().toString().toLatin1();
                query += "</" + i.key().toLatin1() + " xmlns=\"" + ns.toLatin1() + "\"/>";
            }
        }
    }
    query += "</D:prop>";
    query += "</D:propfind>";

    return proppatch(path, query);
}

int QWebdav::proppatch( const QString & path, const QByteArray & query)
{
    QHttpRequestHeader req(QLatin1String("PROPPATCH"), path);

    return davRequest(req, query);
}

void QWebdav::readyRead ( const QHttpResponseHeader & rep )
{
    if (emitListInfo)
        buffer += readAll();
}

void QWebdav::emitListInfos()
{
    QDomDocument multiResponse;

    multiResponse.setContent(buffer, true);
    qDebug()<<" sssssssssssssssssssssssssssssssssssssssss";
    Q_EMIT listInfo(multiResponse.toString());
#if 0
    for ( QDomNode n = multiResponse.documentElement().firstChild();
          !n.isNull(); n = n.nextSibling())
    {
        QDomElement thisResponse = n.toElement();

        if (thisResponse.isNull())
            continue;

        QWebdavUrlInfo info(thisResponse);

        if (!info.isValid())
            continue;

        hasResponse = true;
        emit listInfo(info);
    }
#endif
}

void QWebdav::responseHeaderReceived( const QHttpResponseHeader & resp )
{
    QHttpRequestHeader req = currentRequest();
    QString method = req.method().toUpper();

    if (method == QLatin1String("PROPFIND") || method == QLatin1String("SEARCH"))
        emitListInfo = true;
}

void QWebdav::requestFinished( int id, bool error )
{
    if (emitListInfo && !error)
        emitListInfos();
    buffer.clear();
    emitListInfo = false;
}

void QWebdav::setupHeaders(QHttpRequestHeader & req, quint64 size)
{
    req.setValue(QLatin1String("Host"), host);
    req.setValue(QLatin1String("Connection"), QLatin1String("Keep-Alive"));
    if (size) {
        req.setContentLength(size);
        req.setContentType(QLatin1String("text/xml; charset=utf-8"));
    }
}

int QWebdav::davRequest(QHttpRequestHeader & req, const QByteArray & data)
{
    setupHeaders(req, data.size());
    return request(req, data);
}

int QWebdav::davRequest(QHttpRequestHeader & req, QIODevice * data)
{
    setupHeaders(req, data->size());
    return request(req, data);
}

int QWebdav::mkdir ( const QString & dir )
{
    QHttpRequestHeader req(QLatin1String("MKCOL"), dir);

    return davRequest(req);
}

int QWebdav::copy ( const QString & oldname, const QString & newname, bool overwrite)
{
    QHttpRequestHeader req(QLatin1String("COPY"), oldname);

    req.setValue(QLatin1String("Destination"), newname);
    req.setValue(QLatin1String("Depth"), QLatin1String("infinity"));
    req.setValue(QLatin1String("Overwrite"), overwrite ? QLatin1String("T") : QLatin1String("F"));
    return davRequest(req);
}

int QWebdav::rename ( const QString & oldname, const QString & newname, bool overwrite)
{
    return move(oldname, newname, overwrite);
}

int QWebdav::move ( const QString & oldname, const QString & newname, bool overwrite)
{
    QHttpRequestHeader req(QLatin1String("MOVE"), oldname);

    req.setValue(QLatin1String("Destination"), newname);
    req.setValue(QLatin1String("Depth"), QLatin1String("infinity"));
    req.setValue(QLatin1String("Overwrite"), overwrite ? QLatin1String("T") : QLatin1String("F"));
    return davRequest(req);
}

int QWebdav::rmdir ( const QString & dir )
{
    return remove(dir);
}

int QWebdav::remove ( const QString & path )
{
    QHttpRequestHeader req(QLatin1String("DELETE"), path);

    return davRequest(req);
}


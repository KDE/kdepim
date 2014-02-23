/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  webdav access based on QWebDav Copyright (C) 2009-2010 Corentin Chary <corentin.chary@gmail.com>

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
#include "webdavjob.h"
#include <QBuffer>

using namespace PimCommon;
QNetworkReply *WebDavJob::accountInfo(const QString &dir)
{
    WebDavJob::PropNames query;
    QStringList props;

    props << QLatin1String("quota-available-bytes");
    props << QLatin1String("quota-used-bytes");
    props << QLatin1String("quota");
    props << QLatin1String("quotaused");
    query[QLatin1String("DAV:")] = props;

    return propfind(dir, query, 0);
}

QNetworkReply *WebDavJob::list ( const QString & dir)
{
    WebDavJob::PropNames query;
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

QNetworkReply *WebDavJob::search ( const QString & path, const QString & q )
{
    QByteArray query = "<?xml version=\"1.0\"?>\r\n";

    query.append( "<D:searchrequest xmlns:D=\"DAV:\">\r\n" );
    query.append( q.toUtf8() );
    query.append( "</D:searchrequest>\r\n" );

    QNetworkRequest req;
    req.setUrl(path);

    return davRequest(QLatin1String("SEARCH"), req);
}

QNetworkReply *WebDavJob::put ( const QString & path, QIODevice * data )
{
    QNetworkRequest req;
    req.setUrl(QUrl(path));

    return davRequest(QLatin1String("PUT"), req, data);
}

QNetworkReply *WebDavJob::put ( const QString & path, QByteArray & data )
{
    QBuffer buffer(&data);

    return put(path, &buffer);
}

QNetworkReply *WebDavJob::propfind( const QString &path, const WebDavJob::PropNames & props, int depth)
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

QNetworkReply *WebDavJob::propfind( const QString & path, const QByteArray & query, int depth )
{
    QNetworkRequest req;
    QUrl url(path);
    req.setUrl(url);

    QString value;

    if (depth == 2)
        value = QLatin1String("infinity");
    else
        value = QString::fromLatin1("%1").arg(depth);
    req.setRawHeader(QByteArray("Depth"), value.toUtf8());
    return davRequest(QLatin1String("PROPFIND"), req, query);
}

QNetworkReply *WebDavJob::proppatch( const QString & path, const WebDavJob::PropValues & props)
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

QNetworkReply *WebDavJob::proppatch( const QString & path, const QByteArray & query)
{
    QNetworkRequest req;
    req.setUrl(QUrl(path));

    return davRequest(QLatin1String("PROPPATCH"), req, query);
}

void WebDavJob::setupHeaders(QNetworkRequest & req, quint64 size)
{
    req.setRawHeader(QByteArray("Connection"), QByteArray("Keep-Alive"));
    if (size > 0) {
        req.setHeader(QNetworkRequest::ContentLengthHeader, QVariant(size));
        req.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(QLatin1String("text/xml; charset=utf-8")));
    }
}

QNetworkReply *WebDavJob::davRequest(const QString & reqVerb,  QNetworkRequest & req, const QByteArray & data)
{
    QByteArray dataClone(data);
    QBuffer *buffer = new QBuffer(this);
    buffer->setData(dataClone);
    return davRequest(reqVerb, req, buffer);
}

QNetworkReply *WebDavJob::davRequest(const QString & reqVerb,  QNetworkRequest & req, QIODevice * data)
{
    setupHeaders(req, data->size());
    return mNetworkAccessManager->sendCustomRequest(req, reqVerb.toUtf8(), data);
}

QNetworkReply *WebDavJob::mkdir ( const QUrl & dir )
{
    QNetworkRequest req;
    req.setUrl(QUrl(dir));
    return davRequest(QLatin1String("MKCOL"), req);
}

QNetworkReply *WebDavJob::copy ( const QString & oldname, const QString & newname, bool overwrite)
{
    QNetworkRequest req;

    req.setUrl(QUrl(oldname));
    req.setRawHeader(QByteArray("Destination"), newname.toUtf8());
    req.setRawHeader(QByteArray("Depth"), QByteArray("infinity"));
    req.setRawHeader(QByteArray("Overwrite"), QByteArray(overwrite ? "T" : "F"));
    return davRequest(QLatin1String("COPY"), req);
}

QNetworkReply *WebDavJob::rename ( const QString & oldname, const QString & newname, bool overwrite)
{
    return move(oldname, newname, overwrite);
}

QNetworkReply *WebDavJob::move ( const QString & oldname, const QString & newname, bool overwrite)
{
    QNetworkRequest req;
    req.setUrl(QUrl(oldname));

    req.setRawHeader(QByteArray("Destination"), newname.toUtf8());
    req.setRawHeader(QByteArray("Depth"), QByteArray("infinity"));
    req.setRawHeader(QByteArray("Overwrite"), QByteArray(overwrite ? "T" : "F"));
    return davRequest(QLatin1String("MOVE"), req);
}

QNetworkReply *WebDavJob::rmdir ( const QUrl &url )
{
    return remove(url);
}

QNetworkReply *WebDavJob::remove ( const QUrl & url )
{
    QNetworkRequest req;
    req.setUrl(url);
    return davRequest(QLatin1String("DELETE"), req);
}


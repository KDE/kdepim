// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
    Based on the davjob:
    Copyright (C) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef __kio_xmlrpcjob_h__
#define __kio_xmlrpcjob_h__

#include <kurl.h>

#include <qstring.h>
#include <qvaluelist.h>
#include <qdom.h>

#include <kio/jobclasses.h>
#include <kio/global.h>

namespace KIO {

  class XMLRPCResult;

/**
 * The transfer job pumps data into and/or out of a Slave.
 * Data is sent to the slave on request of the slave ( dataReq ).
 * If data coming from the slave can not be handled, the
 * reading of data from the slave should be suspended.
 * @see KIO::xmlrpcCall()
 * @since 3.4
 */
class XmlrpcJob : public TransferJob {
Q_OBJECT

  public:
    /** Indicates the response type of the call
     */
    enum XMLRPCResponseType {
      XMLRPCMessageResponse,
      XMLRPCFaultResponse,
      XMLRPCUnknownResponse
    };
    /**
     * Use KIO::xmlrpcPropFind(), KIO::xmlrpcPropPatch() and
     * KIO::xmlrpcSearch() to create a new XmlrpcJob.
     */
    XmlrpcJob( const KURL& url, const QString& method, const QValueList<QVariant> &params, bool showProgressInfo );
    virtual XmlrpcJob::~XmlrpcJob();
    /**
     * Returns the response as a QDomDocument.
     * @return the response document
     */
    QValueList<QVariant> &response() { return m_response; }
    /**
     * Returns the type of the response.
     * @return the type of the response
     */
    XMLRPCResponseType responseType() const { return m_responseType; }

    static QString markupCall( const QString &cmd, const QValueList<QVariant> &args );
  protected slots:
    virtual void slotFinished();
    virtual void slotData( const QByteArray &data);

  protected:
    static QString marshal( const QVariant &arg );
    static QVariant demarshal( const QDomElement &e );
    
    static bool isMessageResponse( const QDomDocument &doc );
    static bool isFaultResponse( const QDomDocument &doc );

    static XMLRPCResult parseMessageResponse( const QDomDocument &doc );
    static XMLRPCResult parseFaultResponse( const QDomDocument &doc );

    
  private:
    class XmlrpcJobPrivate;
    XmlrpcJobPrivate *d;
    QString m_str_response;
    QValueList<QVariant> m_response;
    XMLRPCResponseType m_responseType;
};

/**
 * Creates a new XmlrpcJob that calls a remote @p method of the API at the given @p url.
 *
 * @param url the URL of the XML-RPC Interface of the server
 * @param method the name of the method to call
 * @param params the arguments (as QValueList<QVariant>) for the method call. 
 * @param showProgressInfo true to show progress information
 * @return the new XmlrpcJob
 */
XmlrpcJob* xmlrpcCall( const KURL& url, const QString &method, const QValueList<QVariant> &params, bool showProgressInfo = true );

XmlrpcJob* xmlrpcCall( const KURL& url, const QString &method, const QVariant    &arg, bool showProgressInfo = true );
XmlrpcJob* xmlrpcCall( const KURL& url, const QString &method,       int          arg, bool showProgressInfo = true );
XmlrpcJob* xmlrpcCall( const KURL& url, const QString &method,       bool         arg, bool showProgressInfo = true );
XmlrpcJob* xmlrpcCall( const KURL& url, const QString &method,       double       arg, bool showProgressInfo = true );
XmlrpcJob* xmlrpcCall( const KURL& url, const QString &method, const QString     &arg, bool showProgressInfo = true );
XmlrpcJob* xmlrpcCall( const KURL& url, const QString &method, const QCString    &arg, bool showProgressInfo = true );
XmlrpcJob* xmlrpcCall( const KURL& url, const QString &method, const QByteArray  &arg, bool showProgressInfo = true );
XmlrpcJob* xmlrpcCall( const KURL& url, const QString &method, const QDateTime   &arg, bool showProgressInfo = true );
XmlrpcJob* xmlrpcCall( const KURL& url, const QString &method, const QStringList &arg, bool showProgressInfo = true );
template <typename T>
XmlrpcJob* xmlrpcCall( const KURL& url, const QString &method, const QValueList<T>&arg,bool showProgressInfo = true );
}

#endif


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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef __kio_xmlrpcjob_h__
#define __kio_xmlrpcjob_h__

#include <kurl.h>

#include <tqstring.h>
#include <tqvaluelist.h>
#include <tqdom.h>

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
    XmlrpcJob( const KURL& url, const TQString& method,
				       const TQValueList<TQVariant> &params, bool showProgressInfo );
    virtual ~XmlrpcJob();
    /**
     * Returns the response as a TQDomDocument.
     * @return the response document
     */
    TQValueList<TQVariant> &response() { return m_response; }
    /**
     * Returns the type of the response.
     * @return the type of the response
     */
    XMLRPCResponseType responseType() const { return m_responseType; }

    static TQString markupCall( const TQString &cmd,
                               const TQValueList<TQVariant> &args );
  protected slots:
    virtual void slotFinished();
    virtual void slotData( const TQByteArray &data);

  protected:
    static TQString marshal( const TQVariant &arg );
    static TQVariant demarshal( const TQDomElement &e );

    static bool isMessageResponse( const TQDomDocument &doc );
    static bool isFaultResponse( const TQDomDocument &doc );

    static XMLRPCResult parseMessageResponse( const TQDomDocument &doc );
    static XMLRPCResult parseFaultResponse( const TQDomDocument &doc );


  private:
    class XmlrpcJobPrivate;
    XmlrpcJobPrivate *d;
    TQString m_str_response;
    TQValueList<TQVariant> m_response;
    XMLRPCResponseType m_responseType;
};

/**
 * Creates a XmlrpcJob that calls a @p method of the API at the given @p url.
 *
 * @param url the URL of the XML-RPC Interface of the server
 * @param method the name of the method to call
 * @param params the arguments (as TQValueList<TQVariant>) for the method call.
 * @param showProgressInfo true to show progress information
 * @return the new XmlrpcJob
 */
XmlrpcJob* xmlrpcCall( const KURL& url, const TQString &method,
                       const TQValueList<TQVariant> &params,
                       bool showProgressInfo = true );

XmlrpcJob* xmlrpcCall( const KURL& url, const TQString &method,
                       const TQVariant &arg, bool showProgressInfo = true );
XmlrpcJob* xmlrpcCall( const KURL& url, const TQString &method,
                       const TQStringList &arg, bool showProgressInfo = true );
template <typename T>
XmlrpcJob* xmlrpcCall( const KURL& url, const TQString &method,
                       const TQValueList<T>&arg,bool showProgressInfo = true );
}

#endif

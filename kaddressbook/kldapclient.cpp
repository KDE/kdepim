/* kldapclient.cpp - LDAP access
 *      Copyright (C) 2002 Klarälvdalens Datakonsult AB
 *
 *      Author: Steffen Hansen <hansen@kde.org>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <kmdcodec.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qlabel.h>
#include <qfile.h>

#include "kldapclient.h"


QString KLdapObject::toString()
{
  QString result = QString::fromLatin1("\ndn: %1\n").arg( dn );
  for( KLdapAttrMap::Iterator it = attrs.begin(); it != attrs.end(); ++it ) {
    QString attr = it.key();
    for( KLdapAttrValue::Iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2 ) {      
      if( attr == "jpegPhoto" ) {
	QByteArray buf = *it2;
#if 0
	qDebug("Trying to load image from buf with size %d", (*it2).size() );
	QPixmap pix;
	pix.loadFromData( buf, "JPEG" );
	qDebug("Image loaded successfully" );
	QLabel* l = new QLabel(0);
	QFile f("tmp.jpg");
	f.open( IO_WriteOnly );
	f.writeBlock( buf );
	f.close();
	//l->setPixmap( QPixmap("tmp.jpg") );
	//l->show();
#endif
      } else {
	result += QString("%1: %2\n").arg( attr).arg( *it2 );
      }
    }
  }
  return result;  
}

void KLdapObject::clear()
{
  dn = QString::null;
  attrs.clear();
}

void KLdapObject::assign( const KLdapObject& that )
{
  if( &that != this ) {
    dn=that.dn;
    attrs=that.attrs;
  }    
}

KLdapClient::KLdapClient( QObject* parent, const char* name ) : QObject( parent, name ), _job(0), _active(false)
{
}

KLdapClient::~KLdapClient()
{
  cancelQuery();
}

void KLdapClient::setHost( const QString& host )
{
  _host = host;
}

void KLdapClient::setPort( const QString& port )
{
  _port = port;
}

void KLdapClient::setBase( const QString& base )
{
  _base = base;
}

void KLdapClient::setAttrs( const QStringList& attrs )
{
  _attrs = attrs;
}

void KLdapClient::startQuery( const QString& filter )
{
  cancelQuery();
  QString query;
  if( _scope.isEmpty() ) _scope = "sub";
  QString host = _host;
  if( !_port.isEmpty() ) {
    host += ':';
    host += _port;
  }
  if( _attrs.empty() ) {
    query = QString("ldap://%1/%2?*?%3?(%4)").arg( host ).arg( _base ).arg( _scope ).arg( filter );
  } else {
    query = QString("ldap://%1/%2?%3?%4?(%5)").arg( host ).arg( _base )
      .arg( _attrs.join(",") ).arg( _scope ).arg( filter );
  }
  //qDebug("Doing query \"%s\"", query.latin1());
  startParseLDIF();
  _active = true;
  _job = KIO::get( KURL(query), false, false );
  connect( _job, SIGNAL( data( KIO::Job*, const QByteArray& )),
	   this, SLOT( slotData( KIO::Job*, const QByteArray& )));
  connect( _job, SIGNAL( infoMessage( KIO::Job*, const QString& )),
	   this, SLOT( slotInfoMessage( KIO::Job*, const QString& )));
  connect( _job, SIGNAL( result( KIO::Job* ) ),
	   this, SLOT( slotDone() ) );
}

void KLdapClient::cancelQuery()
{
  if( _job ) {
    _job->kill();
    _job=0;
  }
  _active = false;
}

void KLdapClient::slotData( KIO::Job*, const QByteArray& data )
{
  //QString str(data);
  //qDebug( "Got \"%s\"", str.latin1());
  parseLDIF( data );
}

void KLdapClient::slotInfoMessage( KIO::Job*, const QString &info )
{
  //qDebug("Job said \"%s\"", info.latin1());
}

void KLdapClient::slotDone()
{
  endParseLDIF();
  _active = false;
#if 0
  for( QValueList<KLdapObject>::Iterator it = _objects.begin(); it != _objects.end(); ++it ) {
    qDebug( (*it).toString().latin1() );
  }
#endif
  int err = _job->error();
  if( err ) {
    emit error(KIO::buildErrorString( err, QString("%1:%2").arg(_host).arg(_port) ) );
  } 
  emit done();
}

void KLdapClient::startParseLDIF()
{
  //_objects.clear();
  _currentObject.clear();
  _lastAttrName  = 0;
  _lastAttrValue = 0;
  _isBase64 = false;
}

void KLdapClient::endParseLDIF()
{
  if( !_currentObject.dn.isEmpty() ) {
    if( !_lastAttrName.isNull() && !_lastAttrValue.isNull() ) {
      if( _isBase64 ) {
	QByteArray out;
	KCodecs::base64Decode( _lastAttrValue, out );
	//qDebug("_lastAttrValue=\"%s\", output length %d", _lastAttrValue.data(), out.size());
	_currentObject.attrs[_lastAttrName].append( out );
      } else {
	_currentObject.attrs[_lastAttrName].append( _lastAttrValue );
      }
    }
    emit result( _currentObject );
    //_objects << _currentObject;
  }
}

void KLdapClient::parseLDIF( const QByteArray& data )
{  
  //qDebug("%s", data.data());
  if( data.isNull() || data.isEmpty() ) return;
  _buf += QCString( data ); // collect data in buffer
  int nl;
  while( (nl = _buf.find('\n')) != -1 ) {
    // Run through it line by line
    /* FIXME(steffen): This could be a problem
    * with "no newline at end of file" input
    */
    QCString line = _buf.left( nl );
    if( _buf.length() > (unsigned int)(nl+1) ) _buf = _buf.mid( nl+1 );
    else _buf = "";
    if( line.length() > 0 ) {
      if( line[0] == '#' ) {
	// comment
	continue;
      } else if( line[0] == ' ' || line[0] == '\t' ) {
	// continuation of last line
	line = line.stripWhiteSpace();
	//qDebug("Adding \"%s\"", line.data() );
	_lastAttrValue += line;
	continue;
      }
    } else continue;
    int colon = line.find(':');
    if( colon != -1 ) {
      // Found new attribute	
      if( _lastAttrName == "dn" ) {
	// New object, store the current
	if( !_currentObject.dn.isNull() ) {
	  //_objects << _currentObject;
	  emit result( _currentObject );
	  _currentObject.clear();
	}
	_currentObject.dn = _lastAttrValue;
	_lastAttrValue = 0;
	_lastAttrName  = 0;
      } else if( !_lastAttrName.isEmpty()) {
	// Store current value, take care of decoding
	if( _isBase64 ) {
	  QByteArray out;
	  KCodecs::base64Decode( _lastAttrValue, out );
	  //qDebug("_lastAttrValue=\"%s\", output length %d", _lastAttrValue.data(), out.size());
	  _currentObject.attrs[_lastAttrName].append( out );
	} else {
	  _currentObject.attrs[_lastAttrName].append( _lastAttrValue );
	}
      }

      _lastAttrName  = line.left( colon ).stripWhiteSpace();
      //qDebug("Found attr %s", _lastAttrName.data() );
      ++colon;
      if( line[colon] == ':' ) {
	_isBase64 = true;
	//qDebug("BASE64");
	++colon;
      } else {
	//qDebug("UTF8");
	_isBase64 = false;
      }
      _lastAttrValue = line.mid( colon ).stripWhiteSpace();
    }
  }
}
#include "kldapclient.moc"

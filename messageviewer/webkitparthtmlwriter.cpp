/*  -*- c++ -*-
    webkitparthtmlwriter.cpp

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2003 Marc Mutz <mutz@kde.org>
    Copyright (c) 2009 Torgny Nyblom <kde@nyblom.org>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/


#include "webkitparthtmlwriter.h"

#include <kdebug.h>
#include <kwebview.h>
#include <kurl.h>

#include <cassert>
#include <QByteArray>
#include <QWebView>
#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>

WebKitPartHtmlWriter::WebKitPartHtmlWriter( KWebView *view,
                                          QObject * parent, const char * name )
  : QObject( parent ), HtmlWriter(),
    mHtmlView( view ), mState( Ended )
{
  setObjectName( name );
  assert( view );
  mHtmlTimer.setSingleShot( true );
  connect( &mHtmlTimer, SIGNAL(timeout()), SLOT(slotWriteNextHtmlChunk()) );
}

WebKitPartHtmlWriter::~WebKitPartHtmlWriter() {

}

void WebKitPartHtmlWriter::begin( const QString & css ) {
  // The stylesheet is now included CSSHelper::htmlHead()
  Q_UNUSED( css );
  if ( mState != Ended ) {
    kWarning() <<"WebKitPartHtmlWriter: begin() called on non-ended session!";
    reset();
  }

  mEmbeddedPartMap.clear();

  // clear the widget:
  mHtmlView->setUpdatesEnabled( false );
  QPoint point = mHtmlView->page()->mainFrame()->scrollPosition();
  point -= QPoint(0, 10);
  mHtmlView->page()->mainFrame()->setScrollPosition( point );

  mHtmlView->load( QUrl() );
  mState = Begun;
}

void WebKitPartHtmlWriter::end() {
  kWarning( mState != Begun, 5006 ) <<"WebKitPartHtmlWriter: end() called on non-begun or queued session!";
  mHtmlView->setHtml( mHtml, QUrl() );
  mHtmlView->show();
  mHtml.clear();

  resolveCidUrls();

  mHtmlView->setUpdatesEnabled( true );
  mHtmlView->update();
  mState = Ended;
  emit finished();
}

void WebKitPartHtmlWriter::reset() {
  if ( mState != Ended ) {
    mHtmlTimer.stop();
    mHtmlQueue.clear();
    mHtml.clear();
    mState = Begun; // don't run into end()'s warning
    end();
    mState = Ended;
  }
}

void WebKitPartHtmlWriter::write( const QString & str ) {
  kWarning( mState != Begun, 5006 ) <<"WebKitPartHtmlWriter: write() called in Ended or Queued state!";
  mHtml.append( str );
}

void WebKitPartHtmlWriter::queue( const QString & str ) {
  static const uint chunksize = 16384;
  for ( int pos = 0 ; pos < str.length() ; pos += chunksize )
    mHtmlQueue.push_back( str.mid( pos, chunksize ) );
  mState = Queued;
}

void WebKitPartHtmlWriter::flush() {
  slotWriteNextHtmlChunk();
}

void WebKitPartHtmlWriter::slotWriteNextHtmlChunk() {
  if ( mHtmlQueue.empty() ) {
    mState = Begun; // don't run into end()'s warning
    end();
  } else {
    mHtml.append( mHtmlQueue.front() );
    mHtmlQueue.pop_front();
    mHtmlTimer.start( 0 );
  }
}

void WebKitPartHtmlWriter::embedPart( const QByteArray & contentId,
                                      const QString & contentURL ) {
  mEmbeddedPartMap[QString(contentId)] = contentURL;
}

void WebKitPartHtmlWriter::resolveCidUrls()
{
  QWebElement root = mHtmlView->page()->mainFrame()->documentElement();
  QWebElementCollection images = root.findAll( "img" );
  QWebElement image;
  foreach( image, images )
  {
    KUrl url( image.attribute( "src" ) );
    if ( url.protocol() == "cid" )
    {
      EmbeddedPartMap::const_iterator it = mEmbeddedPartMap.constFind( url.path() );
      if ( it != mEmbeddedPartMap.constEnd() ) {
        kDebug() <<"Replacing" << url.prettyUrl() <<" by" << it.value();
        image.setAttribute( "src", it.value() );
      }
    }
  }
}


#include "webkitparthtmlwriter.moc"

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

#include "config-webkit.h"

#include <kdebug.h>
#include <kwebview.h>

//TODO: Port to QWebElement
#include <dom/dom_string.h>
#include <dom/html_document.h>
#include <dom/html_image.h>
#include <dom/html_misc.h>

#include <cassert>
#include <QByteArray>
#include <QWebView>
#include <QWebPage>
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
  if ( mState != Ended ) {
    kWarning() <<"WebKitPartHtmlWriter: begin() called on non-ended session!";
    reset();
  }

  mEmbeddedPartMap.clear();

  // clear the widget:
  mHtmlView->setUpdatesEnabled( false );
#ifdef WEBKIT_BUILD
  mHtmlView->setUpdatesEnabled( false );
  mHtmlView->scroll(0, 0);
#else
  mHtmlView->viewport()->setUpdatesEnabled( false );
  mHtmlView->ensureVisible( 0, 0 );
#endif

#ifdef WEBKIT_BUILD
  mHtmlView->load( QUrl() );
  if ( !css.isEmpty() )
    mCss = css;
#else
  mHtmlView->begin( KUrl() );
  if ( !css.isEmpty() )
    mHtmlView->setUserStyleSheet( css );
#endif
  mState = Begun;
}

void WebKitPartHtmlWriter::end() {
  kWarning( mState != Begun, 5006 ) <<"WebKitPartHtmlWriter: end() called on non-begun or queued session!";
#ifdef WEBKIT_BUILD
  mHtmlView->setHtml( mHtml, QUrl() );
  mHtmlView->show();
  mHtml.clear();
#else
  mHtmlView->end();
#endif

  resolveCidUrls();

#ifdef WEBKIT_BUILD
  mHtmlView->setUpdatesEnabled( true );
#else
  mHtmlView->viewport()->setUpdatesEnabled( true );
  mHtmlView->setUpdatesEnabled( true );
  mHtmlView->viewport()->repaint();
#endif
  mState = Ended;
  emit finished();
}

void WebKitPartHtmlWriter::reset() {
  if ( mState != Ended ) {
    mHtmlTimer.stop();
    mHtmlQueue.clear();
#ifdef WEBKIT_BUILD
    mHtml.clear();
#endif
    mState = Begun; // don't run into end()'s warning
    end();
    mState = Ended;
  }
}

void WebKitPartHtmlWriter::write( const QString & str ) {
  kWarning( mState != Begun, 5006 ) <<"WebKitPartHtmlWriter: write() called in Ended or Queued state!";
#ifdef WEBKIT_BUILD
  mHtml.append( str );
#else
  mHtmlView->write( str );
#endif
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
#ifdef WEBKIT_BUILD
  mHtml.append( mHtmlQueue.front() );
#else
    mHtmlView->write( mHtmlQueue.front() );
#endif
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
#ifdef WEBKIT_BUILD
  kWarning() << "WEBKIT: Disabled code in " << Q_FUNC_INFO;
#else
  DOM::HTMLDocument document = mHtmlView->htmlDocument();
  DOM::HTMLCollection images = document.images();
  for ( DOM::Node node = images.firstItem(); !node.isNull(); node = images.nextItem() ) {
    DOM::HTMLImageElement image( node );
    KUrl url( image.src().string() );
    if ( url.protocol() == "cid" ) {
      EmbeddedPartMap::const_iterator it = mEmbeddedPartMap.constFind( url.path() );
      if ( it != mEmbeddedPartMap.constEnd() ) {
        kDebug() <<"Replacing" << url.prettyUrl() <<" by" << it.value();
        image.setSrc( it.value() );
      }
    }
  }
#endif
}


#include "webkitparthtmlwriter.moc"

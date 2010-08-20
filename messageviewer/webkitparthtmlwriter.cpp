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


#include <config-messageviewer.h>

#include "webkitparthtmlwriter.h"

#include "mailwebview.h"

#include <KDebug>
#include <KUrl>

#include <cassert>
#include <QByteArray>
#include <QWebView>
#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>

using namespace MessageViewer;

WebKitPartHtmlWriter::WebKitPartHtmlWriter( MailWebView * view, QObject * parent )
  : QObject( parent ), HtmlWriter(),
    mHtmlView( view ), mState( Ended )
{
  assert( view );
}

WebKitPartHtmlWriter::~WebKitPartHtmlWriter() {
}

void WebKitPartHtmlWriter::begin( const QString & css ) {
  // The stylesheet is now included CSSHelper::htmlHead()
  Q_UNUSED( css );
  if ( mState != Ended ) {
    kWarning() << "begin() called on non-ended session!";
    reset();
  }

  mEmbeddedPartMap.clear();

  // clear the widget:
  mHtmlView->setUpdatesEnabled( false );
  mHtmlView->scrollUp( 10 );
#ifndef MESSAGEVIEWER_NO_WEBKIT
  // PENDING(marc) push into MailWebView
  mHtmlView->load( QUrl() );
#endif
  mState = Begun;
}

void WebKitPartHtmlWriter::end() {
  if ( mState != Begun ) {
    kWarning() << "Called on non-begun or queued session!";
  }
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
    mHtml.clear();
    mState = Begun; // don't run into end()'s warning
    end();
    mState = Ended;
  }
}

void WebKitPartHtmlWriter::write( const QString & str ) {
  if ( mState != Begun ) {
    kWarning() << "Called in Ended or Queued state!";
  }
  mHtml.append( str );
}

void WebKitPartHtmlWriter::queue( const QString & str ) {
  write( str );
}

void WebKitPartHtmlWriter::flush() {
  mState = Begun; // don't run into end()'s warning
  end();
}

void WebKitPartHtmlWriter::embedPart( const QByteArray & contentId,
                                      const QString & contentURL ) {
  mEmbeddedPartMap[QString(contentId)] = contentURL;
}

void WebKitPartHtmlWriter::resolveCidUrls()
{
  // FIXME: instead of patching around in the HTML source, this should
  // be replaced by a custom QNetworkAccessManager (for QWebView), or
  // virtual loadResource() (for QTextBrowser)
#ifndef MESSAGEVIEWER_NO_WEBKIT
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
        kDebug() << "Replacing" << url.prettyUrl() << "by" << it.value();
        image.setAttribute( "src", it.value() );
      }
    }
  }
#endif
}


#include "webkitparthtmlwriter.moc"

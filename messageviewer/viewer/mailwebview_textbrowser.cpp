/* Copyright 2010 Thomas McGuire <mcguire@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "mailwebview.h"

#include <KDebug>
#include <KActionCollection>

#include <QContextMenuEvent>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QScrollBar>

#include <limits>
#include <cassert>

using namespace boost;
using namespace MessageViewer;

MailWebView::MailWebView( KActionCollection *actionCollection, QWidget *parent )
  : QTextBrowser( parent ), // krazy:exclude=qclasses
    mActionCollection(actionCollection)
{
  setOpenLinks( false );

  connect( this, SIGNAL(highlighted(QString)),
           this, SIGNAL(linkHovered(QString)) );
  connect( this, SIGNAL(anchorClicked(QUrl)),
           this, SIGNAL(linkClicked(QUrl)) );

#ifdef KDEPIM_MOBILE_UI
  setFrameShape( NoFrame );
#endif
}

MailWebView::~MailWebView() {}

bool MailWebView::event( QEvent *event )
{
#ifdef TEMPORARILY_REMOVED
  if ( event->type() == QEvent::ContextMenu ) {
    // Don't call SuperClass::event() here, it will do silly things like selecting the text
    // under the mouse cursor, which we don't want.

    QContextMenuEvent const *contextMenuEvent = static_cast<QContextMenuEvent*>( event );
    const QWebFrame * const frame = page()->currentFrame();
    const QWebHitTestResult hit = frame->hitTestContent( contextMenuEvent->pos() );
    kDebug() << "Right-clicked URL:" << hit.linkUrl();
    emit popupMenu( hit.linkUrl().toString(), QUrl(), mapToGlobal( contextMenuEvent->pos() ) );
    event->accept();
    return true;
  }
#endif
  return QTextBrowser::event( event ); // krazy:exclude=qclasses
}

void MailWebView::scrollDown( int pixels )
{
  if ( QScrollBar * const vsb = verticalScrollBar() )
    vsb->setValue( vsb->value() + pixels );
}

void MailWebView::scrollUp( int pixels )
{
  scrollDown( -pixels );
}

bool MailWebView::isScrolledToBottom() const
{
  const QScrollBar * const vsb = verticalScrollBar();
  return !vsb || vsb->value() == vsb->maximum();
}

void MailWebView::scrollPageDown( int percent )
{
  if ( QScrollBar * const vsb = verticalScrollBar() ) {
      const qint64 height =  vsb->pageStep();
      const qint64 current = vsb->value();
      // do arithmetic in higher precision, and check for overflow:
      const qint64 newPosition = current + height * percent / 100;
      if ( newPosition > std::numeric_limits<int>::max() )
          kWarning() << "new position" << newPosition << "exceeds range of 'int'!";
      vsb->setValue( newPosition );
  }
}

void MailWebView::scrollPageUp( int percent )
{
  scrollPageDown( -percent );
}

QString MailWebView::selectedText() const
{
  return textCursor().selectedText().replace( QChar::ParagraphSeparator, QLatin1Char('\n') );
}

bool MailWebView::hasVerticalScrollBar() const
{
  if ( const QScrollBar * const vsb = verticalScrollBar() )
    return vsb->isVisible();
  else
    return false;
}

double MailWebView::relativePosition() const
{
  if ( const QScrollBar * const vsb = verticalScrollBar() ) {
    const double pos = vsb->value();
    const int height = vsb->maximum();
    return height ? pos / height : 0.0 ;
  } else {
    return 0.0;
  }
}

void MailWebView::scrollToRelativePosition( double pos )
{
  // FIXME: This doesn't work, Qt resets the scrollbar value somewhere in the event handler.
  //        Using a singleshot timer wouldn't work either, since that introduces visible scrolling.
  if ( QScrollBar * const vsb = verticalScrollBar() )
    vsb->setValue( vsb->maximum() * pos );
}

void MailWebView::selectAll()
{
  QTextBrowser::selectAll(); // krazy:exclude=qclasses
}

void MailWebView::clearSelection()
{
  QTextCursor cursor = textCursor();
  cursor.clearSelection();
  setTextCursor( cursor );
}

bool MailWebView::isAttachmentInjectionPoint( const QPoint & global ) const
{
  // this is not needed in the cases we use QTextBrowser, but should eventually be implemented
  kDebug() << "sorry, not implemented";
  Q_UNUSED( global );
  return false;
}

void MailWebView::injectAttachments( const function<QString()> & delayedHtml )
{
  // this is not needed in the cases we use QTextBrowser, but should eventually be implemented
  kDebug() << "sorry, not implemented";
  Q_UNUSED( delayedHtml );
}

void MailWebView::scrollToAnchor( const QString & anchor )
{
  QTextBrowser::scrollToAnchor( anchor ); // krazy:exclude=qclasses
}

bool MailWebView::removeAttachmentMarking( const QString & id )
{
  // this is not needed in the cases we use QTextBrowser, but should eventually be implemented
  kDebug() << "sorry, not implemented";
  Q_UNUSED( id );
  return true;
}

void MailWebView::markAttachment( const QString & id, const QString & style )
{
  // this is not needed in the cases we use QTextBrowser, but should eventually be implemented
  kDebug() << "sorry, not implemented";
  Q_UNUSED( id );
  Q_UNUSED( style );
}

void MailWebView::setHtml( const QString & html, const QUrl & base )
{
  // PENDING(marc) does that make sense?
  setSource( base );
  QTextBrowser::setHtml( html ); // krazy:exclude=qclasses
}

QString MailWebView::htmlSource() const
{
  return toHtml();
}

void MailWebView::setAllowExternalContent( bool allow )
{
#ifdef TEMPORARILY_REMOVED
  // FIXME on WinCE we use a simple QWebView, check if there's an alternative API for it
#ifndef Q_OS_WINCE
    SuperClass::setAllowExternalContent( allow );
#endif
#endif
}

QUrl MailWebView::linkOrImageUrlAt( const QPoint & global ) const
{
  const QTextCursor c = cursorForPosition( viewport()->mapFromGlobal( global ) );
  const QTextCharFormat f = c.charFormat();
  const QString ahref = f.anchorHref();
  if ( ahref.isEmpty() )
      return f.toImageFormat().name();
  else
      return ahref;
}

void MailWebView::setScrollBarPolicy( Qt::Orientation orientation, Qt::ScrollBarPolicy policy )
{
  switch ( orientation ) {
    case Qt::Horizontal:
      setHorizontalScrollBarPolicy( policy );
      break;
    case Qt::Vertical:
      setVerticalScrollBarPolicy( policy );
      break;
    default:
      Q_ASSERT( false );
      break;
  }
}

Qt::ScrollBarPolicy MailWebView::scrollBarPolicy( Qt::Orientation orientation ) const
{
  switch ( orientation ) {
    case Qt::Horizontal:
      return horizontalScrollBarPolicy();
    case Qt::Vertical:
      return verticalScrollBarPolicy();
      break;
    default:
      Q_ASSERT( false );
      return Qt::ScrollBarAsNeeded;
  }
}

bool MailWebView::replaceInnerHtml( const QString & id, const function<QString()> & delayedHtml )
{
#ifdef TEMPORARILY_REMOVED
  QWebElement doc = page()->currentFrame()->documentElement();
  QWebElement tag = doc.findFirst( "*#" + id );
  if ( tag.isNull() ) {
    return false;
  }
  tag.setInnerXml( delayedHtml() );
#endif
  return true;
}

void MailWebView::setElementByIdVisible( const QString & id, bool visible )
{
#ifdef TEMPORARILY_REMOVED
  QWebElement doc = page()->currentFrame()->documentElement();
  QWebElement e = doc.findFirst( "*#" + id );
  Q_ASSERT( !e.isNull() );

  if ( visible ) {
    e.removeAttribute( "display" );
  } else {
    e.setStyleProperty( "display", "none" );
  }
#endif
}

static QTextDocument::FindFlags convert_flags( MailWebView::FindFlags f )
{
    QTextDocument::FindFlags result;
#ifdef TEMPORARILY_REMOVED
    if ( f & MailWebView::FindWrapsAroundDocument )
        result |= QTextDocument::FindWrapsAroundDocument;
#endif
    if ( f & MailWebView::FindBackward )
        result |= QTextDocument::FindBackward;
    if ( f & MailWebView::FindCaseSensitively )
        result |= QTextDocument::FindCaseSensitively;
#ifdef TEMPORARILY_REMOVED
    if ( f & MailWebView::HighlightAllOccurrences )
        result |= QTextDocument::HighlightAllOccurrences;
#endif
    return result;
}

bool MailWebView::findText( const QString & text, FindFlags flags )
{
  return find( text, convert_flags( flags ) );
}

void MailWebView::clearFindSelection()
{
  // not supported
}

void MailWebView::keyReleaseEvent(QKeyEvent* e)
{
  QTextBrowser::keyReleaseEvent(e);
}

void MailWebView::keyPressEvent(QKeyEvent* e)
{
  QTextBrowser::keyPressEvent(e);
}

void MailWebView::wheelEvent (QWheelEvent* e)
{
  QTextBrowser::wheelEvent(e);
}

void MailWebView::hideAccessKeys()
{
}

void MailWebView::slotShowDetails()
{
}

void MailWebView::expandUrl(const KUrl &url)
{
    Q_UNUSED(url)
}

bool MailWebView::isAShortUrl(const KUrl &url) const
{
    Q_UNUSED(url)
    return false;
}

#include "moc_mailwebview.cpp"

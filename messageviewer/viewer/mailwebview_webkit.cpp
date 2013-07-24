/* Copyright 2010 Thomas McGuire <mcguire@kde.org>

   Copyright 2013 Laurent Montel <monte@kde.org>

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
#include "scamdetection/scamdetection.h"

#include <KDebug>
#include <KActionCollection>
#include <KAction>

#include <QCoreApplication>
#include <QContextMenuEvent>
#include <QWebFrame>
#include <QWebElement>
#include <QLabel>
#include <QToolTip>

#include <limits>
#include <cassert>

#ifdef Q_OS_WINCE
typedef QWebView SuperClass;
#else
typedef KWebView SuperClass;
#endif

using namespace boost;
using namespace MessageViewer;

static QString linkElementKey(const QWebElement& element)
{
  if (element.hasAttribute(QLatin1String("href"))) {
    const QUrl url = element.webFrame()->baseUrl().resolved(element.attribute(QLatin1String("href")));
    QString linkKey (url.toString());
    if (element.hasAttribute(QLatin1String("target"))) {
      linkKey += QLatin1Char('+');
      linkKey += element.attribute(QLatin1String("target"));
    }
    return linkKey;
  }
  return QString();
}


static bool isHiddenElement(const QWebElement& element)
{
  // width property set to less than zero
  if (element.hasAttribute(QLatin1String("width")) && element.attribute(QLatin1String("width")).toInt() < 1) {
    return true;
  }

  // height property set to less than zero
  if (element.hasAttribute(QLatin1String("height")) && element.attribute(QLatin1String("height")).toInt() < 1) {
    return true;
  }

  // visibility set to 'hidden' in the element itself or its parent elements.
  if (element.styleProperty(QLatin1String("visibility"),QWebElement::ComputedStyle).compare(QLatin1String("hidden"), Qt::CaseInsensitive) == 0) {
    return true;
  }

  // display set to 'none' in the element itself or its parent elements.
  if (element.styleProperty(QLatin1String("display"),QWebElement::ComputedStyle).compare(QLatin1String("none"), Qt::CaseInsensitive) == 0) {
    return true;
  }

  return false;
}

static bool isEditableElement(QWebPage* page)
{
  const QWebFrame* frame = (page ? page->currentFrame() : 0);
  QWebElement element = (frame ? frame->findFirstElement(QLatin1String(":focus")) : QWebElement());
  if (!element.isNull()) {
     const QString tagName(element.tagName());
     if (tagName.compare(QLatin1String("textarea"), Qt::CaseInsensitive) == 0) {
       return true;
     }
     const QString type(element.attribute(QLatin1String("type")).toLower());
     if (tagName.compare(QLatin1String("input"), Qt::CaseInsensitive) == 0
        && (type.isEmpty() || type == QLatin1String("text") || type == QLatin1String("password"))) {
       return true;
     }
     if (element.evaluateJavaScript(QLatin1String("this.isContentEditable")).toBool()) {
       return true;
     }
  }
  return false;
}

static void handleDuplicateLinkElements(const QWebElement& element, QHash<QString, QChar>* dupLinkList, QChar* accessKey)
{
  if (element.tagName().compare(QLatin1String("A"), Qt::CaseInsensitive) == 0) {
    const QString linkKey (linkElementKey(element));
    // kDebug() << "LINK KEY:" << linkKey;
    if (dupLinkList->contains(linkKey)) {
       // kDebug() << "***** Found duplicate link element:" << linkKey << endl;
       *accessKey = dupLinkList->value(linkKey);
    } else if (!linkKey.isEmpty()) {
       dupLinkList->insert(linkKey, *accessKey);
    }
    if (linkKey.isEmpty())
       *accessKey = QChar();
  }
}


MailWebView::MailWebView( KActionCollection *actionCollection, QWidget *parent )
    : SuperClass( parent ), mScamDetection(new ScamDetection), mActionCollection(actionCollection)
{
  page()->setLinkDelegationPolicy( QWebPage::DelegateAllLinks );
  settings()->setAttribute( QWebSettings::JavascriptEnabled, false );
  settings()->setAttribute( QWebSettings::JavaEnabled, false );
  settings()->setAttribute( QWebSettings::PluginsEnabled, false );
  connect( page(), SIGNAL(linkHovered(QString,QString,QString)),
           this,   SIGNAL(linkHovered(QString,QString,QString)) );
  connect(this, SIGNAL(loadStarted()), this, SLOT(hideAccessKeys()));
  connect(mScamDetection, SIGNAL(messageMayBeAScam()), this, SIGNAL(messageMayBeAScam()));
  connect(page(), SIGNAL(scrollRequested(int,int,QRect)), this, SLOT(hideAccessKeys()));
}

MailWebView::~MailWebView()
{
    delete mScamDetection;
}

bool MailWebView::event( QEvent *event )
{
  if ( event->type() == QEvent::ContextMenu ) {
    // Don't call SuperClass::event() here, it will do silly things like selecting the text
    // under the mouse cursor, which we don't want.

    QContextMenuEvent const *contextMenuEvent = static_cast<QContextMenuEvent*>( event );
    const QWebFrame * const frame = page()->currentFrame();
    const QWebHitTestResult hit = frame->hitTestContent( contextMenuEvent->pos() );
    kDebug() << "Right-clicked URL:" << hit.linkUrl();

#ifdef Q_OS_WINCE
    if ( !hit.linkUrl().isEmpty() )
#endif
      emit popupMenu( hit.linkUrl(), ((hit.pixmap().isNull()) ? QUrl() : hit.imageUrl()), mapToGlobal( contextMenuEvent->pos() ) );
    event->accept();
    return true;
  }
  return SuperClass::event( event );
}

void MailWebView::scrollDown( int pixels )
{
  QPoint point = page()->mainFrame()->scrollPosition();
  point.ry() += pixels;
  page()->mainFrame()->setScrollPosition( point );
}

void MailWebView::scrollUp( int pixels )
{
  scrollDown( -pixels );
}

bool MailWebView::isScrolledToBottom() const
{
  const int pos = page()->mainFrame()->scrollBarValue( Qt::Vertical );
  const int max = page()->mainFrame()->scrollBarMaximum( Qt::Vertical );
  return pos == max;
}

void MailWebView::scrollPageDown( int percent )
{
  const qint64 height =  page()->viewportSize().height();
  const qint64 current = page()->mainFrame()->scrollBarValue( Qt::Vertical );
  // do arithmetic in higher precision, and check for overflow:
  const qint64 newPosition = current + height * percent / 100;
  if ( newPosition > std::numeric_limits<int>::max() )
      kWarning() << "new position" << newPosition << "exceeds range of 'int'!";
  page()->mainFrame()->setScrollBarValue( Qt::Vertical, newPosition );
}

void MailWebView::scrollPageUp( int percent )
{
  scrollPageDown( -percent );
}

QString MailWebView::selectedText() const
{
//TODO HTML selection
/* settings()->setAttribute( QWebSettings::JavascriptEnabled, true );
  QString textSelected = page()->currentFrame()->evaluateJavaScript(
    "var span = document.createElement( 'SPAN' ); span.appendChild( window.getSelection().getRangeAt(0).cloneContents() );
  ).toString();
  settings()->setAttribute( QWebSettings::JavascriptEnabled, false );

  return textSelected;
*/
  return SuperClass::selectedText();
}

bool MailWebView::hasVerticalScrollBar() const
{
  return page()->mainFrame()->scrollBarGeometry( Qt::Vertical ).isValid();
}

double MailWebView::relativePosition() const
{
  if ( hasVerticalScrollBar() ) {
    const double pos = page()->mainFrame()->scrollBarValue( Qt::Vertical );
    const int height = page()->mainFrame()->scrollBarMaximum( Qt::Vertical );
    return height ? pos / height : 0.0 ;
  } else {
    return 0.0;
  }
}

void MailWebView::scrollToRelativePosition( double pos )
{
  // FIXME: This doesn't work, Qt resets the scrollbar value somewhere in the event handler.
  //        Using a singleshot timer wouldn't work either, since that introduces visible scrolling.
  const int max = page()->mainFrame()->scrollBarMaximum( Qt::Vertical );
  page()->currentFrame()->setScrollBarValue( Qt::Vertical, max * pos );
}

void MailWebView::selectAll()
{
  page()->triggerAction( QWebPage::SelectAll );
}

void MailWebView::clearSelection()
{
  //This is an ugly hack to remove the selection, I found no other way to do it with QWebView
  QMouseEvent event(QEvent::MouseButtonPress, QPoint( 10, 10 ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  QCoreApplication::sendEvent( page(), &event );
  QMouseEvent event2(QEvent::MouseButtonRelease, QPoint( 10, 10 ), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
  QCoreApplication::sendEvent( page(), &event2 );
}

// Checks if the given node has a child node that is a DIV which has an ID attribute
// with the value specified here
static bool has_parent_div_with_id( const QWebElement & start, const QString & id )
{
  if ( start.isNull() )
    return false;

  if ( start.tagName().toLower() == QLatin1String("div") ) {
      if ( start.attribute( QLatin1String("id"), QString() ) == id )
      return true;
  }

  return has_parent_div_with_id( start.parent(), id );
}

bool MailWebView::isAttachmentInjectionPoint( const QPoint & global ) const
{
  // for QTextBrowser, can be implemented as 'return false'
  const QPoint local = page()->view()->mapFromGlobal( global );
  const QWebHitTestResult hit = page()->currentFrame()->hitTestContent( local );
  return has_parent_div_with_id( hit.enclosingBlockElement(), QLatin1String("attachmentInjectionPoint") );
}

void MailWebView::injectAttachments( const function<QString()> & delayedHtml )
{
  // for QTextBrowser, can be implemented empty
  QWebElement doc = page()->currentFrame()->documentElement();
  QWebElement injectionPoint = doc.findFirst( QLatin1String("*#attachmentInjectionPoint") );
  if( injectionPoint.isNull() )
    return;

  const QString html = delayedHtml();
  if ( html.isEmpty() )
    return;

  assert( injectionPoint.tagName().toLower() == QLatin1String("div") );
  injectionPoint.setInnerXml( html );
}

void MailWebView::scrollToAnchor( const QString & anchor )
{
  QWebElement doc = page()->mainFrame()->documentElement();
  QWebElement link = doc.findFirst( QLatin1String("a[name=") + anchor +QLatin1Char(']') );
  if ( link.isNull() ) {
    return;
  }

  const int linkPos = link.geometry().bottom();
  const int viewerPos  = page()->mainFrame()->scrollPosition().y();
  link.setFocus();
  page()->mainFrame()->scroll(0, linkPos - viewerPos );

}

bool MailWebView::removeAttachmentMarking( const QString & id )
{
  QWebElement doc = page()->mainFrame()->documentElement();
  QWebElement attachmentDiv = doc.findFirst( QLatin1String("*#") + id );
  if ( attachmentDiv.isNull() )
    return false;
  attachmentDiv.removeAttribute( QLatin1String("style") );
  return true;
}

void MailWebView::markAttachment( const QString & id, const QString & style )
{
  QWebElement doc = page()->mainFrame()->documentElement();
  QWebElement attachmentDiv = doc.findFirst( QLatin1String("*#") + id );
  if ( !attachmentDiv.isNull() ) {
    attachmentDiv.setAttribute(QLatin1String( "style"), style );
  }
}

void MailWebView::setHtml( const QString & html, const QUrl & base )
{
  SuperClass::setHtml( html, base );
}

QString MailWebView::htmlSource() const
{
  return page()->mainFrame()->documentElement().toOuterXml();
}

void MailWebView::setAllowExternalContent( bool allow )
{
  // FIXME on WinCE we use a simple QWebView, check if there's an alternative API for it
#ifndef Q_OS_WINCE
    SuperClass::setAllowExternalContent( allow );
#endif
}

QUrl MailWebView::linkOrImageUrlAt( const QPoint & global ) const
{
  const QPoint local = page()->view()->mapFromGlobal( global );
  const QWebHitTestResult hit = page()->currentFrame()->hitTestContent( local );
  if ( !hit.linkUrl().isEmpty() )
    return hit.linkUrl();
  else if ( !hit.imageUrl().isEmpty() )
    return hit.imageUrl();
  else
    return QUrl();
}


void MailWebView::setScrollBarPolicy( Qt::Orientation orientation, Qt::ScrollBarPolicy policy )
{
  page()->mainFrame()->setScrollBarPolicy( orientation, policy );
}

Qt::ScrollBarPolicy MailWebView::scrollBarPolicy( Qt::Orientation orientation ) const
{
  return page()->mainFrame()->scrollBarPolicy( orientation );
}


bool MailWebView::replaceInnerHtml( const QString & id, const function<QString()> & delayedHtml )
{
  QWebElement doc = page()->currentFrame()->documentElement();
  QWebElement tag = doc.findFirst( QLatin1String("*#") + id );
  if ( tag.isNull() ) {
    return false;
  }
  tag.setInnerXml( delayedHtml() );
  return true;
}

void MailWebView::setElementByIdVisible( const QString & id, bool visible )
{
  QWebElement doc = page()->currentFrame()->documentElement();
  QWebElement e = doc.findFirst( QLatin1String("*#") + id );
  Q_ASSERT( !e.isNull() );

  if ( visible ) {
    e.removeAttribute( QLatin1String("display") );
  } else {
    e.setStyleProperty( QLatin1String("display"), QLatin1String("none") );
  }
}

static QWebPage::FindFlags convert_flags( MailWebView::FindFlags f )
{
  QWebPage::FindFlags result;
  if ( f & MailWebView::FindWrapsAroundDocument )
    result |= QWebPage::FindWrapsAroundDocument;
  if ( f & MailWebView::FindBackward )
    result |= QWebPage::FindBackward;
  if ( f & MailWebView::FindCaseSensitively )
    result |= QWebPage::FindCaseSensitively;
  if ( f & MailWebView::HighlightAllOccurrences )
    result |= QWebPage::HighlightAllOccurrences;
  return result;
}

bool MailWebView::findText( const QString & text, FindFlags flags )
{
  return SuperClass::findText( text, convert_flags( flags ) );
}

void MailWebView::clearFindSelection()
{
  //WEBKIT: TODO: Find a way to unselect last selection
  // http://bugreports.qt.nokia.com/browse/QTWEBKIT-80
  SuperClass::findText( QString(), QWebPage::HighlightAllOccurrences );
}

void MailWebView::keyReleaseEvent(QKeyEvent*e)
{
  if (GlobalSettings::self()->accessKeyEnabled() && mAccessKeyActivated == PreActivated) {
    // Activate only when the CTRL key is pressed and released by itself.
    if (e->key() == Qt::Key_Control && e->modifiers() == Qt::NoModifier) {
      showAccessKeys();
      mAccessKeyActivated = Activated;
    } else {
      mAccessKeyActivated = NotActivated;
    }
  }
  SuperClass::keyReleaseEvent(e);
}

void MailWebView::keyPressEvent(QKeyEvent*e)
{
  if (e && hasFocus()) {
    if (GlobalSettings::self()->accessKeyEnabled()) {
       if (mAccessKeyActivated == Activated) {
          if (checkForAccessKey(e)) {
             hideAccessKeys();
             e->accept();
             return;
          }
          hideAccessKeys();
       } else if (e->key() == Qt::Key_Control && e->modifiers() == Qt::ControlModifier && !isEditableElement(page())) {
          mAccessKeyActivated = PreActivated; // Only preactive here, it will be actually activated in key release.
       }
     }
  }
  SuperClass::keyPressEvent(e);
}

void MailWebView::wheelEvent(QWheelEvent* e)
{
  if (GlobalSettings::self()->accessKeyEnabled() && mAccessKeyActivated == PreActivated && (e->modifiers() & Qt::ControlModifier)) {
    mAccessKeyActivated = NotActivated;
  }
  SuperClass::wheelEvent(e);
}

bool MailWebView::checkForAccessKey(QKeyEvent *event)
{
  if (mAccessKeyLabels.isEmpty())
    return false;
  QString text = event->text();
  if (text.isEmpty())
     return false;
  QChar key = text.at(0).toUpper();
  bool handled = false;
  if (mAccessKeyNodes.contains(key)) {
    QWebElement element = mAccessKeyNodes[key];
    QPoint p = element.geometry().center();
    QWebFrame *frame = element.webFrame();
    Q_ASSERT(frame);
    do {
      p -= frame->scrollPosition();
      frame = frame->parentFrame();
    } while (frame && frame != page()->mainFrame());
    QMouseEvent pevent(QEvent::MouseButtonPress, p, Qt::LeftButton, 0, 0);
    QCoreApplication::sendEvent(this, &pevent);
    QMouseEvent revent(QEvent::MouseButtonRelease, p, Qt::LeftButton, 0, 0);
    QCoreApplication::sendEvent(this, &revent);
    handled = true;
  }
  return handled;
}

void MailWebView::hideAccessKeys()
{
  if (!mAccessKeyLabels.isEmpty()) {
    for (int i = 0, count = mAccessKeyLabels.count(); i < count; ++i) {
      QLabel *label = mAccessKeyLabels[i];
      label->hide();
      label->deleteLater();
    }
    mAccessKeyLabels.clear();
    mAccessKeyNodes.clear();
    mDuplicateLinkElements.clear();
    mAccessKeyActivated = NotActivated;
    update();
  }
}


void MailWebView::showAccessKeys()
{
    QList<QChar> unusedKeys;
    for (char c = 'A'; c <= 'Z'; ++c) {
        unusedKeys << QLatin1Char(c);
    }
    for (char c = '0'; c <= '9'; ++c) {
        unusedKeys << QLatin1Char(c);
    }
    Q_FOREACH(QAction*act, mActionCollection->actions()) {
        KAction *a = qobject_cast<KAction*>(act);
        if(a) {
            const KShortcut shortCut = a->shortcut();
            if(!shortCut.isEmpty()) {
                Q_FOREACH(const QChar& c, unusedKeys) {
                    if(shortCut.conflictsWith(QKeySequence(c))) {
                        unusedKeys.removeOne(c);
                    }
                }
            }
        }
    }

    QList<QWebElement> unLabeledElements;
    QRect viewport = QRect(page()->mainFrame()->scrollPosition(), page()->viewportSize());
    const QString selectorQuery (QLatin1String("a[href],"
                                               "area,"
                                               "button:not([disabled]),"
                                               "input:not([disabled]):not([hidden]),"
                                               "label[for],"
                                               "legend,"
                                               "select:not([disabled]),"
                                               "textarea:not([disabled])"));
    QList<QWebElement> result = page()->mainFrame()->findAllElements(selectorQuery).toList();

    // Priority first goes to elements with accesskey attributes
    Q_FOREACH (const QWebElement& element, result) {
        const QRect geometry = element.geometry();
        if (geometry.size().isEmpty() || !viewport.contains(geometry.topLeft())) {
            continue;
        }
        if (isHiddenElement(element)) {
            continue;    // Do not show access key for hidden elements...
        }
        const QString accessKeyAttribute (element.attribute(QLatin1String("accesskey")).toUpper());
        if (accessKeyAttribute.isEmpty()) {
            unLabeledElements.append(element);
            continue;
        }
        QChar accessKey;
        for (int i = 0; i < accessKeyAttribute.count(); i+=2) {
            const QChar &possibleAccessKey = accessKeyAttribute[i];
            if (unusedKeys.contains(possibleAccessKey)) {
                accessKey = possibleAccessKey;
                break;
            }
        }
        if (accessKey.isNull()) {
            unLabeledElements.append(element);
            continue;
        }

        handleDuplicateLinkElements(element, &mDuplicateLinkElements, &accessKey);
        if (!accessKey.isNull()) {
            unusedKeys.removeOne(accessKey);
            makeAccessKeyLabel(accessKey, element);
        }
    }


    // Pick an access key first from the letters in the text and then from the
    // list of unused access keys
    Q_FOREACH (const QWebElement &element, unLabeledElements) {
        const QRect geometry = element.geometry();
        if (unusedKeys.isEmpty()
            || geometry.size().isEmpty()
            || !viewport.contains(geometry.topLeft()))
            continue;
        QChar accessKey;
        const QString text = element.toPlainText().toUpper();
        for (int i = 0; i < text.count(); ++i) {
            const QChar &c = text.at(i);
            if (unusedKeys.contains(c)) {
                accessKey = c;
                break;
            }
        }
        if (accessKey.isNull())
            accessKey = unusedKeys.takeFirst();

        handleDuplicateLinkElements(element, &mDuplicateLinkElements, &accessKey);
        if (!accessKey.isNull()) {
            unusedKeys.removeOne(accessKey);
            makeAccessKeyLabel(accessKey, element);
        }
    }

    mAccessKeyActivated = (mAccessKeyLabels.isEmpty() ? Activated : NotActivated);
}

void MailWebView::makeAccessKeyLabel(const QChar &accessKey, const QWebElement &element)
{
    QLabel *label = new QLabel(this);
    QFont font (label->font());
    font.setBold(true);
    label->setFont(font);
    label->setText(accessKey);
    label->setPalette(QToolTip::palette());
    label->setAutoFillBackground(true);
    label->setFrameStyle(QFrame::Box | QFrame::Plain);
    QPoint point = element.geometry().center();
    point -= page()->mainFrame()->scrollPosition();
    label->move(point);
    label->show();
    point.setX(point.x() - label->width() / 2);
    label->move(point);
    mAccessKeyLabels.append(label);
    mAccessKeyNodes.insertMulti(accessKey, element);
}

void MailWebView::scamCheck()
{
    QWebFrame *mainFrame = page()->mainFrame();
    mScamDetection->scanPage(mainFrame);
}

void MailWebView::slotShowDetails()
{
    mScamDetection->showDetails();
}

void MailWebView::saveMainFrameScreenshotInFile(const QString &filename)
{
    QWebFrame *frame = page()->mainFrame();
    QImage image(frame->contentsSize(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    frame->documentElement().render(&painter);
    painter.end();
    image.save(filename);
}

#include "mailwebview.moc"

/*  -*- c++ -*-
    urlhandlermanager.cpp

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2002-2003 Klarälvdalens Datakonsult AB
    Copyright (c) 2003      Marc Mutz <mutz@kde.org>

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


#include "urlhandlermanager.h"

#include "interfaces/urlhandler.h"
#include "interfaces/bodyparturlhandler.h"
#include "partnodebodypart.h"
#include "kmreaderwin.h"
#include "nodehelper.h"

#include "stringutil.h"
#include "stl_util.h"
#include <kurl.h>

#include <kmime/kmime_content.h>

#include <QProcess>
#include <algorithm>

#include <QScrollArea>

using std::for_each;
using std::remove;
using std::find;
using namespace MailViewer;

MailViewer::URLHandlerManager * MailViewer::URLHandlerManager::self = 0;

namespace {
  class KMailProtocolURLHandler : public MailViewer::URLHandler {
  public:
    KMailProtocolURLHandler() : MailViewer::URLHandler() {}
    ~KMailProtocolURLHandler() {}

    bool handleClick( const KUrl &, KMReaderWin * ) const;
    bool handleContextMenuRequest( const KUrl & url, const QPoint &, KMReaderWin * ) const {
      return url.protocol() == "kmail";
    }
    QString statusBarMessage( const KUrl &, KMReaderWin * ) const;
  };

  class ExpandCollapseQuoteURLManager : public MailViewer::URLHandler {
  public:
    ExpandCollapseQuoteURLManager() : MailViewer::URLHandler() {}
    ~ExpandCollapseQuoteURLManager() {}

    bool handleClick( const KUrl &, KMReaderWin * ) const;
    bool handleContextMenuRequest( const KUrl &, const QPoint &, KMReaderWin * ) const {
      return false;
    }
    QString statusBarMessage( const KUrl &, KMReaderWin * ) const;

  };

  class SMimeURLHandler : public MailViewer::URLHandler {
  public:
    SMimeURLHandler() : MailViewer::URLHandler() {}
    ~SMimeURLHandler() {}

    bool handleClick( const KUrl &, KMReaderWin * ) const;
    bool handleContextMenuRequest( const KUrl &, const QPoint &, KMReaderWin * ) const {
      return false;
    }
    QString statusBarMessage( const KUrl &, KMReaderWin * ) const;
  };

  class MailToURLHandler : public MailViewer::URLHandler {
  public:
    MailToURLHandler() : MailViewer::URLHandler() {}
    ~MailToURLHandler() {}

    bool handleClick( const KUrl &, KMReaderWin * ) const { return false; }
    bool handleContextMenuRequest( const KUrl &, const QPoint &, KMReaderWin * ) const {
      return false;
    }
    QString statusBarMessage( const KUrl &, KMReaderWin * ) const;
  };

  class HtmlAnchorHandler : public MailViewer::URLHandler {
  public:
    HtmlAnchorHandler() : MailViewer::URLHandler() {}
    ~HtmlAnchorHandler() {}

    bool handleClick( const KUrl &, KMReaderWin * ) const;
    bool handleContextMenuRequest( const KUrl &, const QPoint &, KMReaderWin * ) const {
      return false;
    }
    QString statusBarMessage( const KUrl &, KMReaderWin * ) const { return QString(); }
  };

  class AttachmentURLHandler : public MailViewer::URLHandler {
  public:
    AttachmentURLHandler() : MailViewer::URLHandler() {}
    ~AttachmentURLHandler() {}

    bool handleClick( const KUrl &, KMReaderWin * ) const;
    bool handleContextMenuRequest( const KUrl &, const QPoint &, KMReaderWin * ) const;
    QString statusBarMessage( const KUrl &, KMReaderWin * ) const;
  };

  class ShowAuditLogURLHandler : public MailViewer::URLHandler {
  public:
      ShowAuditLogURLHandler() : MailViewer::URLHandler() {}
      ~ShowAuditLogURLHandler() {}

      bool handleClick( const KUrl &, KMReaderWin * ) const;
      bool handleContextMenuRequest( const KUrl &, const QPoint &, KMReaderWin * ) const;
      QString statusBarMessage( const KUrl &, KMReaderWin * ) const;
  };

  class FallBackURLHandler : public MailViewer::URLHandler {
  public:
    FallBackURLHandler() : MailViewer::URLHandler() {}
    ~FallBackURLHandler() {}

    bool handleClick( const KUrl &, KMReaderWin * ) const;
    bool handleContextMenuRequest( const KUrl &, const QPoint &, KMReaderWin * ) const;
    QString statusBarMessage( const KUrl & url, KMReaderWin * ) const {
      return url.prettyUrl();
    }
  };

} // anon namespace


//
//
// BodyPartURLHandlerManager
//
//

class MailViewer::URLHandlerManager::BodyPartURLHandlerManager : public MailViewer::URLHandler {
public:
  BodyPartURLHandlerManager() : MailViewer::URLHandler() {}
  ~BodyPartURLHandlerManager();

  bool handleClick( const KUrl &, KMReaderWin * ) const;
  bool handleContextMenuRequest( const KUrl &, const QPoint &, KMReaderWin * ) const;
  QString statusBarMessage( const KUrl &, KMReaderWin * ) const;

  void registerHandler( const Interface::BodyPartURLHandler * handler );
  void unregisterHandler( const Interface::BodyPartURLHandler * handler );

private:
  typedef QVector<const Interface::BodyPartURLHandler*> BodyPartHandlerList;
  BodyPartHandlerList mHandlers;
};

MailViewer::URLHandlerManager::BodyPartURLHandlerManager::~BodyPartURLHandlerManager() {
  for_each( mHandlers.begin(), mHandlers.end(),
	    DeleteAndSetToZero<Interface::BodyPartURLHandler>() );
}

void MailViewer::URLHandlerManager::BodyPartURLHandlerManager::registerHandler( const Interface::BodyPartURLHandler * handler ) {
  if ( !handler )
    return;
  unregisterHandler( handler ); // don't produce duplicates
  mHandlers.push_back( handler );
}

void MailViewer::URLHandlerManager::BodyPartURLHandlerManager::unregisterHandler( const Interface::BodyPartURLHandler * handler ) {
  // don't delete them, only remove them from the list!
  mHandlers.erase( remove( mHandlers.begin(), mHandlers.end(), handler ), mHandlers.end() );
}

static KMime::Content * partNodeFromXKMailUrl( const KUrl & url, KMReaderWin * w, QString * path ) {
  assert( path );

  if ( !w || url.protocol() != "x-kmail" )
    return 0;
  const QString urlPath = url.path();

  // urlPath format is: /bodypart/<random number>/<part id>/<path>

  kDebug() <<"BodyPartURLHandler: urlPath == \"" << urlPath <<"\"";
  if ( !urlPath.startsWith( QLatin1String("/bodypart/") ) )
    return 0;

  const QStringList urlParts = urlPath.mid( 10 ).split( '/' );
  if ( urlParts.size() != 3 )
    return 0;
  KMime::ContentIndex index( urlParts[1] );
  *path = KUrl::fromPercentEncoding( urlParts[2].toLatin1() );
  return w->nodeForContentIndex( index );
}

bool MailViewer::URLHandlerManager::BodyPartURLHandlerManager::handleClick( const KUrl & url, KMReaderWin * w ) const {
/*FIXME(Andras) port it
  QString path;
  partNode * node = partNodeFromXKMailUrl( url, w, &path );
  if ( !node )
    return false;
  KMMessage *msg = w->message();
  if ( !msg ) return false;
  Callback callback( msg, w );
  MailViewer::PartNodeBodyPart part( *node, w->overrideCodec() );
  for ( BodyPartHandlerList::const_iterator it = mHandlers.begin() ; it != mHandlers.end() ; ++it )
    if ( (*it)->handleClick( &part, path, callback ) )
      return true;
      */
      kWarning() <<"FIXME PORT bool MailViewer::URLHandlerManager::BodyPartURLHandlerManager::handleClick( const KUrl & url, KMReaderWin * w ) const ";

  return false;
}

bool MailViewer::URLHandlerManager::BodyPartURLHandlerManager::handleContextMenuRequest( const KUrl & url, const QPoint & p, KMReaderWin * w ) const {
  QString path;
  KMime::Content * node = partNodeFromXKMailUrl( url, w, &path );
  if ( !node )
    return false;

  MailViewer::PartNodeBodyPart part( node, w->overrideCodec() );
  for ( BodyPartHandlerList::const_iterator it = mHandlers.begin() ; it != mHandlers.end() ; ++it )
    if ( (*it)->handleContextMenuRequest( &part, path, p ) )
      return true;
  return false;
}

QString MailViewer::URLHandlerManager::BodyPartURLHandlerManager::statusBarMessage( const KUrl & url, KMReaderWin * w ) const {
  QString path;
  KMime::Content * node = partNodeFromXKMailUrl( url, w, &path );
  if ( !node )
    return QString();

  MailViewer::PartNodeBodyPart part( node, w->overrideCodec() );
  for ( BodyPartHandlerList::const_iterator it = mHandlers.begin() ; it != mHandlers.end() ; ++it ) {
    const QString msg = (*it)->statusBarMessage( &part, path );
    if ( !msg.isEmpty() )
      return msg;
  }
  return QString();
}

//
//
// URLHandlerManager
//
//

MailViewer::URLHandlerManager::URLHandlerManager() {
  registerHandler( new KMailProtocolURLHandler() );
  registerHandler( new ExpandCollapseQuoteURLManager() );
  registerHandler( new SMimeURLHandler() );
  registerHandler( new MailToURLHandler() );
  registerHandler( new HtmlAnchorHandler() );
  registerHandler( new AttachmentURLHandler() );
  registerHandler( mBodyPartURLHandlerManager = new BodyPartURLHandlerManager() );
  registerHandler( new ShowAuditLogURLHandler() );
  registerHandler( new FallBackURLHandler() );
}

MailViewer::URLHandlerManager::~URLHandlerManager() {
  for_each( mHandlers.begin(), mHandlers.end(),
	    DeleteAndSetToZero<URLHandler>() );
}

void MailViewer::URLHandlerManager::registerHandler( const URLHandler * handler ) {
  if ( !handler )
    return;
  unregisterHandler( handler ); // don't produce duplicates
  mHandlers.push_back( handler );
}

void MailViewer::URLHandlerManager::unregisterHandler( const URLHandler * handler ) {
  // don't delete them, only remove them from the list!
  mHandlers.erase( remove( mHandlers.begin(), mHandlers.end(), handler ), mHandlers.end() );
}

void MailViewer::URLHandlerManager::registerHandler( const Interface::BodyPartURLHandler * handler ) {
  if ( mBodyPartURLHandlerManager )
    mBodyPartURLHandlerManager->registerHandler( handler );
}

void MailViewer::URLHandlerManager::unregisterHandler( const Interface::BodyPartURLHandler * handler ) {
  if ( mBodyPartURLHandlerManager )
    mBodyPartURLHandlerManager->unregisterHandler( handler );
}

bool MailViewer::URLHandlerManager::handleClick( const KUrl & url, KMReaderWin * w ) const {
  for ( HandlerList::const_iterator it = mHandlers.begin() ; it != mHandlers.end() ; ++it )
    if ( (*it)->handleClick( url, w ) )
      return true;
  return false;
}

bool MailViewer::URLHandlerManager::handleContextMenuRequest( const KUrl & url, const QPoint & p, KMReaderWin * w ) const {
  for ( HandlerList::const_iterator it = mHandlers.begin() ; it != mHandlers.end() ; ++it )
    if ( (*it)->handleContextMenuRequest( url, p, w ) )
      return true;
  return false;
}

QString MailViewer::URLHandlerManager::statusBarMessage( const KUrl & url, KMReaderWin * w ) const {
  for ( HandlerList::const_iterator it = mHandlers.begin() ; it != mHandlers.end() ; ++it ) {
    const QString msg = (*it)->statusBarMessage( url, w );
    if ( !msg.isEmpty() )
      return msg;
  }
  return QString();
}


//
//
// URLHandler
//
//

#include <ui/messagebox.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <khtml_part.h>

#include <QString>

namespace {
  bool KMailProtocolURLHandler::handleClick( const KUrl & url, KMReaderWin * w ) const {
    if ( url.protocol() == "kmail" ) {
      if ( !w )
        return false;

      if ( url.path() == "showHTML" ) {
        w->setHtmlOverride( !w->htmlOverride() );
        w->update( KMReaderWin::Force );
        return true;
      }

      if ( url.path() == "loadExternal" ) {
        w->setHtmlLoadExtOverride( !w->htmlLoadExtOverride() );
        w->update( KMReaderWin::Force );
        return true;
      }

      if ( url.path() == "decryptMessage" ) {
        w->setDecryptMessageOverwrite( true );
        w->update( KMReaderWin::Force );
        return true;
      }

      if ( url.path() == "showSignatureDetails" ) {
        w->setShowSignatureDetails( true );
        w->update( KMReaderWin::Force );
        return true;
      }

      if ( url.path() == "hideSignatureDetails" ) {
        w->setShowSignatureDetails( false );
        w->update( KMReaderWin::Force );
        return true;
      }

      if ( url.path() == "showAttachmentQuicklist" ) {
        w->saveRelativePosition();
        w->setShowAttachmentQuicklist( true );
        w->update( KMReaderWin::Force );
        return true;
      }

      if ( url.path() == "hideAttachmentQuicklist" ) {
        w->saveRelativePosition();
        w->setShowAttachmentQuicklist( false );
        w->update( KMReaderWin::Force );
        return true;
      }
    }
    return false;
  }

  QString KMailProtocolURLHandler::statusBarMessage( const KUrl & url, KMReaderWin * ) const {
    if ( url.protocol() == "kmail" )
    {
      if ( url.path() == "showHTML" )
        return i18n("Turn on HTML rendering for this message.");
      if ( url.path() == "loadExternal" )
        return i18n("Load external references from the Internet for this message.");
      if ( url.path() == "goOnline" )
        return i18n("Work online.");
      if ( url.path() == "decryptMessage" )
        return i18n("Decrypt message.");
      if ( url.path() == "showSignatureDetails" )
        return i18n("Show signature details.");
      if ( url.path() == "hideSignatureDetails" )
        return i18n("Hide signature details.");
      if ( url.path() == "showAttachmentQuicklist" )
        return i18n( "Show attachment list." );
      if ( url.path() == "hideAttachmentQuicklist" )
        return i18n( "Hide attachment list." );
    }
    return QString() ;
  }
}

namespace {

  bool ExpandCollapseQuoteURLManager::handleClick(
      const KUrl & url, KMReaderWin * w ) const
  {
    //  kmail:levelquote/?num      -> the level quote to collapse.
    //  kmail:levelquote/?-num      -> expand all levels quote.
    if ( url.protocol() == "kmail" && url.path()=="levelquote" )
    {
      QString levelStr= url.query().mid( 1,url.query().length() );
      bool isNumber;
      int levelQuote= levelStr.toInt(&isNumber);
      if ( isNumber )
        w->slotLevelQuote( levelQuote );
      return true;
    }
    return false;
  }
  QString ExpandCollapseQuoteURLManager::statusBarMessage(
      const KUrl & url, KMReaderWin * ) const
  {
      if ( url.protocol() == "kmail" && url.path() == "levelquote" )
      {
        QString query= url.query();
        if ( query.length()>=2 ) {
          if ( query[ 1 ] =='-'  ) {
            return i18n("Expand all quoted text.");
          }
          else {
            return i18n("Collapse quoted text.");
          }
        }
      }
      return QString() ;
  }

}

// defined in kmreaderwin.cpp...
extern bool foundSMIMEData( const QString aUrl, QString & displayName,
                            QString & libName, QString & keyId );

namespace {
  bool SMimeURLHandler::handleClick( const KUrl & url, KMReaderWin * w ) const {
    if ( !url.hasRef() )
      return false;
    QString displayName, libName, keyId;
    if ( !foundSMIMEData( url.path() + '#' +
                          QUrl::fromPercentEncoding( url.ref().toLatin1() ),
                          displayName, libName, keyId ) )
      return false;
    QStringList lst;
    lst << "-query" << keyId;
    if ( !QProcess::startDetached( "kleopatra",lst) )
      KMessageBox::error( w, i18n("Could not start certificate manager. "
                                  "Please check your installation."),
                             i18n("KMail Error") );
    return true;
  }

  QString SMimeURLHandler::statusBarMessage( const KUrl & url, KMReaderWin * ) const {
    QString displayName, libName, keyId;
    if ( !foundSMIMEData( url.path() + '#' +
                          QUrl::fromPercentEncoding( url.ref().toLatin1() ),
                          displayName, libName, keyId ) )
      return QString();
    return i18n("Show certificate 0x%1", keyId );
  }
}

namespace {
  bool HtmlAnchorHandler::handleClick( const KUrl & url, KMReaderWin * w ) const {
    if ( url.hasHost() || url.path() != "/" || !url.hasRef() )
      return false;
    if ( w && !w->htmlPart()->gotoAnchor( url.ref() ) )
      static_cast<QScrollArea*>( w->htmlPart()->widget() )->ensureVisible( 0, 0 );
    return true;
  }
}

namespace {
  QString MailToURLHandler::statusBarMessage( const KUrl & url, KMReaderWin * ) const {
    if ( url.protocol() != "mailto" )
      return QString();
    return MailViewer::StringUtil::decodeMailtoUrl( url.url() );
  }
}

namespace {
  bool AttachmentURLHandler::handleClick( const KUrl & url, KMReaderWin * w ) const {
    if ( !w || !w->message() )
      return false;
    KMime::Content *node = w->nodeFromUrl( url );
    if ( !node )
      return false;
    // PENDING(romain_kdab) : replace with toLocalFile() ?
    w->openAttachment( node, url.path() );
    return true;
  }

  bool AttachmentURLHandler::handleContextMenuRequest( const KUrl & url, const QPoint & p, KMReaderWin * w ) const {
    if ( !w || !w->message() )
      return false;
    KMime::Content *node = w->nodeFromUrl( url );
    if ( !node )
      return false;
    // PENDING(romain_kdab) : replace with toLocalFile() ?
    /*FIXME(Andras) port it
    w->showAttachmentPopup( id, url.path(), p );
    */
    return true;
  }

  QString AttachmentURLHandler::statusBarMessage( const KUrl & url, KMReaderWin * w ) const {
    if ( !w || !w->message() )
      return QString();
    KMime::Content * node = w->nodeFromUrl( url );
    if ( !node )
      return QString();
    QString name = NodeHelper::fileName( node );
    if ( !name.isEmpty() )
      return i18n( "Attachment: %1", name );
    return i18n( "Attachment #%1 (unnamed)", node->index().toString() );
  }
}

namespace {
  static QString extractAuditLog( const KUrl & url ) {
    if ( url.protocol() != "kmail" || url.path() != "showAuditLog" )
      return QString();
    assert( !url.queryItem( "log" ).isEmpty() );
    return url.queryItem( "log" );
  }

  bool ShowAuditLogURLHandler::handleClick( const KUrl & url, KMReaderWin * w ) const {
    const QString auditLog = extractAuditLog( url );
    if ( auditLog.isEmpty() )
        return false;
    Kleo::MessageBox::auditLog( w, auditLog );
    return true;
  }

  bool ShowAuditLogURLHandler::handleContextMenuRequest( const KUrl & url, const QPoint &, KMReaderWin * w ) const {
    Q_UNUSED( w );
    // disable RMB for my own links:
    return !extractAuditLog( url ).isEmpty();
  }

  QString ShowAuditLogURLHandler::statusBarMessage( const KUrl & url, KMReaderWin * ) const {
    if ( extractAuditLog( url ).isEmpty() )
      return QString();
    else
      return i18n("Show GnuPG Audit Log for this operation");
  }
}

namespace {
  bool FallBackURLHandler::handleClick( const KUrl & url, KMReaderWin * w ) const {
    if ( w )
      w->emitUrlClicked( url, Qt::LeftButton );
    return true;
  }

  bool FallBackURLHandler::handleContextMenuRequest( const KUrl & url, const QPoint & p, KMReaderWin * w ) const {
    if ( w )
      w->emitPopupMenu( url, p );
    return true;
  }
}

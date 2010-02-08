
/*  -*- c++ -*-
    urlhandlermanager.cpp

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2003      Marc Mutz <mutz@kde.org>
    Copyright (C) 2002-2003, 2009 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
    Copyright (c) 2009 Andras Mantia <andras@kdab.net>

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
#include "viewer_p.h"
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
using namespace MessageViewer;

URLHandlerManager * URLHandlerManager::self = 0;

namespace {
  class KMailProtocolURLHandler : public URLHandler {
  public:
    KMailProtocolURLHandler() : URLHandler() {}
    ~KMailProtocolURLHandler() {}

    bool handleClick( const KUrl &, ViewerPrivate * ) const;
    bool handleContextMenuRequest( const KUrl & url, const QPoint &, ViewerPrivate * ) const {
      return url.protocol() == "kmail";
    }
    QString statusBarMessage( const KUrl &, ViewerPrivate * ) const;
  };

  class ExpandCollapseQuoteURLManager : public URLHandler {
  public:
    ExpandCollapseQuoteURLManager() : URLHandler() {}
    ~ExpandCollapseQuoteURLManager() {}

    bool handleClick( const KUrl &, ViewerPrivate * ) const;
    bool handleContextMenuRequest( const KUrl &, const QPoint &, ViewerPrivate * ) const {
      return false;
    }
    QString statusBarMessage( const KUrl &, ViewerPrivate * ) const;

  };

  class SMimeURLHandler : public URLHandler {
  public:
    SMimeURLHandler() : URLHandler() {}
    ~SMimeURLHandler() {}

    bool handleClick( const KUrl &, ViewerPrivate * ) const;
    bool handleContextMenuRequest( const KUrl &, const QPoint &, ViewerPrivate * ) const {
      return false;
    }
    QString statusBarMessage( const KUrl &, ViewerPrivate * ) const;
  };

  class MailToURLHandler : public URLHandler {
  public:
    MailToURLHandler() : URLHandler() {}
    ~MailToURLHandler() {}

    bool handleClick( const KUrl &, ViewerPrivate * ) const { return false; }
    bool handleContextMenuRequest( const KUrl &, const QPoint &, ViewerPrivate * ) const {
      return false;
    }
    QString statusBarMessage( const KUrl &, ViewerPrivate * ) const;
  };

  class HtmlAnchorHandler : public URLHandler {
  public:
    HtmlAnchorHandler() : URLHandler() {}
    ~HtmlAnchorHandler() {}

    bool handleClick( const KUrl &, ViewerPrivate * ) const;
    bool handleContextMenuRequest( const KUrl &, const QPoint &, ViewerPrivate * ) const {
      return false;
    }
    QString statusBarMessage( const KUrl &, ViewerPrivate * ) const { return QString(); }
  };

  class AttachmentURLHandler : public URLHandler {
  public:
    AttachmentURLHandler() : URLHandler() {}
    ~AttachmentURLHandler() {}

    bool handleClick( const KUrl &, ViewerPrivate * ) const;
    bool handleContextMenuRequest( const KUrl &, const QPoint &, ViewerPrivate * ) const;
    QString statusBarMessage( const KUrl &, ViewerPrivate * ) const;
  private:
    KMime::Content* nodeForUrl( const KUrl &url, ViewerPrivate *w ) const;
    bool attachmentIsInHeader( const KUrl &url ) const;
  };

  class ShowAuditLogURLHandler : public URLHandler {
  public:
      ShowAuditLogURLHandler() : URLHandler() {}
      ~ShowAuditLogURLHandler() {}

      bool handleClick( const KUrl &, ViewerPrivate * ) const;
      bool handleContextMenuRequest( const KUrl &, const QPoint &, ViewerPrivate * ) const;
      QString statusBarMessage( const KUrl &, ViewerPrivate * ) const;
  };

  class FallBackURLHandler : public URLHandler {
  public:
    FallBackURLHandler() : URLHandler() {}
    ~FallBackURLHandler() {}

    bool handleClick( const KUrl &, ViewerPrivate * ) const;
    bool handleContextMenuRequest( const KUrl &, const QPoint &, ViewerPrivate * ) const;
    QString statusBarMessage( const KUrl & url, ViewerPrivate * ) const {
      return url.prettyUrl();
    }
  };

} // anon namespace


//
//
// BodyPartURLHandlerManager
//
//

class URLHandlerManager::BodyPartURLHandlerManager : public URLHandler {
public:
  BodyPartURLHandlerManager() : URLHandler() {}
  ~BodyPartURLHandlerManager();

  bool handleClick( const KUrl &, ViewerPrivate * ) const;
  bool handleContextMenuRequest( const KUrl &, const QPoint &, ViewerPrivate * ) const;
  QString statusBarMessage( const KUrl &, ViewerPrivate * ) const;

  void registerHandler( const Interface::BodyPartURLHandler * handler );
  void unregisterHandler( const Interface::BodyPartURLHandler * handler );

private:
  typedef QVector<const Interface::BodyPartURLHandler*> BodyPartHandlerList;
  BodyPartHandlerList mHandlers;
};

URLHandlerManager::BodyPartURLHandlerManager::~BodyPartURLHandlerManager() {
  for_each( mHandlers.begin(), mHandlers.end(),
	    DeleteAndSetToZero<Interface::BodyPartURLHandler>() );
}

void URLHandlerManager::BodyPartURLHandlerManager::registerHandler( const Interface::BodyPartURLHandler * handler ) {
  if ( !handler )
    return;
  unregisterHandler( handler ); // don't produce duplicates
  mHandlers.push_back( handler );
}

void URLHandlerManager::BodyPartURLHandlerManager::unregisterHandler( const Interface::BodyPartURLHandler * handler ) {
  // don't delete them, only remove them from the list!
  mHandlers.erase( remove( mHandlers.begin(), mHandlers.end(), handler ), mHandlers.end() );
}

static KMime::Content * partNodeFromXKMailUrl( const KUrl & url, ViewerPrivate * w, QString * path ) {
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

bool URLHandlerManager::BodyPartURLHandlerManager::handleClick( const KUrl & url, ViewerPrivate * w ) const {
  QString path;
  KMime::Content * node = partNodeFromXKMailUrl( url, w, &path );
  if ( !node )
    return false;

  PartNodeBodyPart part( w->messageItem(), node, w->nodeHelper(), w->overrideCodec() );
  for ( BodyPartHandlerList::const_iterator it = mHandlers.begin() ; it != mHandlers.end() ; ++it ) {
    if ( (*it)->handleClick( &part, path ) )
      return true;
  }

  return false;
}

bool URLHandlerManager::BodyPartURLHandlerManager::handleContextMenuRequest( const KUrl & url, const QPoint & p, ViewerPrivate * w ) const {
  QString path;
  KMime::Content * node = partNodeFromXKMailUrl( url, w, &path );
  if ( !node )
    return false;

  PartNodeBodyPart part( w->messageItem(), node, w->nodeHelper(), w->overrideCodec() );
  for ( BodyPartHandlerList::const_iterator it = mHandlers.begin() ; it != mHandlers.end() ; ++it )
    if ( (*it)->handleContextMenuRequest( &part, path, p ) )
      return true;
  return false;
}

QString URLHandlerManager::BodyPartURLHandlerManager::statusBarMessage( const KUrl & url, ViewerPrivate * w ) const {
  QString path;
  KMime::Content * node = partNodeFromXKMailUrl( url, w, &path );
  if ( !node )
    return QString();

  PartNodeBodyPart part( w->messageItem(), node, w->nodeHelper(), w->overrideCodec() );
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

URLHandlerManager::URLHandlerManager() {
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

URLHandlerManager::~URLHandlerManager() {
  for_each( mHandlers.begin(), mHandlers.end(),
	    DeleteAndSetToZero<URLHandler>() );
}

void URLHandlerManager::registerHandler( const URLHandler * handler ) {
  if ( !handler )
    return;
  unregisterHandler( handler ); // don't produce duplicates
  mHandlers.push_back( handler );
}

void URLHandlerManager::unregisterHandler( const URLHandler * handler ) {
  // don't delete them, only remove them from the list!
  mHandlers.erase( remove( mHandlers.begin(), mHandlers.end(), handler ), mHandlers.end() );
}

void URLHandlerManager::registerHandler( const Interface::BodyPartURLHandler * handler ) {
  if ( mBodyPartURLHandlerManager )
    mBodyPartURLHandlerManager->registerHandler( handler );
}

void URLHandlerManager::unregisterHandler( const Interface::BodyPartURLHandler * handler ) {
  if ( mBodyPartURLHandlerManager )
    mBodyPartURLHandlerManager->unregisterHandler( handler );
}

bool URLHandlerManager::handleClick( const KUrl & url, ViewerPrivate * w ) const {
  for ( HandlerList::const_iterator it = mHandlers.begin() ; it != mHandlers.end() ; ++it )
    if ( (*it)->handleClick( url, w ) )
      return true;
  return false;
}

bool URLHandlerManager::handleContextMenuRequest( const KUrl & url, const QPoint & p, ViewerPrivate * w ) const {
  for ( HandlerList::const_iterator it = mHandlers.begin() ; it != mHandlers.end() ; ++it )
    if ( (*it)->handleContextMenuRequest( url, p, w ) )
      return true;
  return false;
}

QString URLHandlerManager::statusBarMessage( const KUrl & url, ViewerPrivate * w ) const {
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
  bool KMailProtocolURLHandler::handleClick( const KUrl & url, ViewerPrivate * w ) const {
    if ( url.protocol() == "kmail" ) {
      if ( !w )
        return false;

      if ( url.path() == "showHTML" ) {
        w->setHtmlOverride( !w->htmlOverride() );
        w->update( Viewer::Force );
        return true;
      }

      if ( url.path() == "loadExternal" ) {
        w->setHtmlLoadExtOverride( !w->htmlLoadExtOverride() );
        w->update( Viewer::Force );
        return true;
      }

      if ( url.path() == "decryptMessage" ) {
        w->setDecryptMessageOverwrite( true );
        w->update( Viewer::Force );
        return true;
      }

      if ( url.path() == "showSignatureDetails" ) {
        w->setShowSignatureDetails( true );
        w->update( Viewer::Force );
        return true;
      }

      if ( url.path() == "hideSignatureDetails" ) {
        w->setShowSignatureDetails( false );
        w->update( Viewer::Force );
        return true;
      }

      if ( url.path() == "showAttachmentQuicklist" ) {
        w->saveRelativePosition();
        w->setShowAttachmentQuicklist( true );
        w->update( Viewer::Force );
        return true;
      }

      if ( url.path() == "hideAttachmentQuicklist" ) {
        w->saveRelativePosition();
        w->setShowAttachmentQuicklist( false );
        w->update( Viewer::Force );
        return true;
      }
    }
    return false;
  }

  QString KMailProtocolURLHandler::statusBarMessage( const KUrl & url, ViewerPrivate * ) const {
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
      const KUrl & url, ViewerPrivate * w ) const
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
      const KUrl & url, ViewerPrivate * ) const
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

bool foundSMIMEData( const QString aUrl,
                     QString& displayName,
                     QString& libName,
                     QString& keyId )
{
  static QString showCertMan("showCertificate#");
  displayName = "";
  libName = "";
  keyId = "";
  int i1 = aUrl.indexOf( showCertMan );
  if( -1 < i1 ) {
    i1 += showCertMan.length();
    int i2 = aUrl.indexOf(" ### ", i1);
    if( i1 < i2 )
    {
      displayName = aUrl.mid( i1, i2-i1 );
      i1 = i2+5;
      i2 = aUrl.indexOf(" ### ", i1);
      if( i1 < i2 )
      {
        libName = aUrl.mid( i1, i2-i1 );
        i2 += 5;

        keyId = aUrl.mid( i2 );
        /*
        int len = aUrl.length();
        if( len > i2+1 ) {
          keyId = aUrl.mid( i2, 2 );
          i2 += 2;
          while( len > i2+1 ) {
            keyId += ':';
            keyId += aUrl.mid( i2, 2 );
            i2 += 2;
          }
        }
        */
      }
    }
  }
  return !keyId.isEmpty();
}


namespace {
  bool SMimeURLHandler::handleClick( const KUrl & url, ViewerPrivate * w ) const {
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
      KMessageBox::error( w->mMainWindow, i18n("Could not start certificate manager. "
                                  "Please check your installation."),
                             i18n("KMail Error") );
    return true;
  }

  QString SMimeURLHandler::statusBarMessage( const KUrl & url, ViewerPrivate * ) const {
    QString displayName, libName, keyId;
    if ( !foundSMIMEData( url.path() + '#' +
                          QUrl::fromPercentEncoding( url.ref().toLatin1() ),
                          displayName, libName, keyId ) )
      return QString();
    return i18n("Show certificate 0x%1", keyId );
  }
}

namespace {
  bool HtmlAnchorHandler::handleClick( const KUrl & url, ViewerPrivate * w ) const {
    if ( url.hasHost() || url.path() != "/" || !url.hasRef() )
      return false;
    kWarning() << "WEBKIT: Disabled code in " << Q_FUNC_INFO;
#if 0
    if ( w && !w->htmlPart()->gotoAnchor( url.ref() ) )
      static_cast<QScrollArea*>( w->htmlPart()->widget() )->ensureVisible( 0, 0 );
#endif
    return true;
  }
}

namespace {
  QString MailToURLHandler::statusBarMessage( const KUrl & url, ViewerPrivate * ) const {
    if ( url.protocol() != "mailto" )
      return QString();
    return StringUtil::decodeMailtoUrl( url.url() );
  }
}

namespace {
 KMime::Content* AttachmentURLHandler::nodeForUrl( const KUrl &url, ViewerPrivate *w ) const
 {
   if ( !w || !w->mMessage )
     return 0;
   if ( url.protocol() != "attachment" )
     return 0;

   KMime::ContentIndex index( url.path() );
   KMime::Content * node = w->nodeForContentIndex( index );
   return node;
 }

 bool AttachmentURLHandler::attachmentIsInHeader( const KUrl &url ) const
 {
   bool inHeader = false;
   const QString place = url.queryItem( "place" ).toLower();
   if ( place != QString::null ) {
     inHeader = ( place == "header" );
   }
   return inHeader;
 }

  bool AttachmentURLHandler::handleClick( const KUrl & url, ViewerPrivate * w ) const
  {
    KMime::Content *node = nodeForUrl( url, w );
    if ( !node )
      return false;

    const bool inHeader = attachmentIsInHeader( url );
    const bool shouldShowDialog = !w->nodeHelper()->isNodeDisplayedEmbedded( node ) || !inHeader;
    if ( inHeader )
      w->scrollToAttachment( node );
    if ( shouldShowDialog )
       // PENDING(romain_kdab) : replace with toLocalFile() ?
       w->openAttachment( node, w->nodeHelper()->tempFileUrlFromNode( node ).path() );

    return true;
  }

  bool AttachmentURLHandler::handleContextMenuRequest( const KUrl & url, const QPoint & p, ViewerPrivate * w ) const
  {
    KMime::Content *node = nodeForUrl( url, w );
    if ( !node )
      return false;
    // PENDING(romain_kdab) : replace with toLocalFile() ?
    w->showAttachmentPopup( node, w->nodeHelper()->tempFileUrlFromNode( node ).path(), p );
    return true;
  }

  QString AttachmentURLHandler::statusBarMessage( const KUrl & url, ViewerPrivate * w ) const
  {
    KMime::Content *node = nodeForUrl( url, w );
    if ( !node )
      return QString();
    QString name = NodeHelper::fileName( node );
    if ( !name.isEmpty() )
      return i18n( "Attachment: %1", name );
    else if ( dynamic_cast<KMime::Message*>( node ) ) {
      return i18n( "Encapsulated Message (Subject: %1)",
                   node->header<KMime::Headers::Subject>()->asUnicodeString() );
    }
    return i18n( "Unnamed attachment" );
  }
}

namespace {
  static QString extractAuditLog( const KUrl & url ) {
    if ( url.protocol() != "kmail" || url.path() != "showAuditLog" )
      return QString();
    assert( !url.queryItem( "log" ).isEmpty() );
    return url.queryItem( "log" );
  }

  bool ShowAuditLogURLHandler::handleClick( const KUrl & url, ViewerPrivate * w ) const {
    const QString auditLog = extractAuditLog( url );
    if ( auditLog.isEmpty() )
        return false;
    Kleo::MessageBox::auditLog( w->mMainWindow, auditLog );
    return true;
  }

  bool ShowAuditLogURLHandler::handleContextMenuRequest( const KUrl & url, const QPoint &, ViewerPrivate * w ) const {
    Q_UNUSED( w );
    // disable RMB for my own links:
    return !extractAuditLog( url ).isEmpty();
  }

  QString ShowAuditLogURLHandler::statusBarMessage( const KUrl & url, ViewerPrivate * ) const {
    if ( extractAuditLog( url ).isEmpty() )
      return QString();
    else
      return i18n("Show GnuPG Audit Log for this operation");
  }
}

namespace {
  bool FallBackURLHandler::handleClick( const KUrl & url, ViewerPrivate * w ) const {
    if ( w )
      w->emitUrlClicked( url, Qt::LeftButton );
    return true;
  }

  bool FallBackURLHandler::handleContextMenuRequest( const KUrl & url, const QPoint & p, ViewerPrivate * w ) const {
    if ( w )
      w->emitPopupMenu( url, p );
    return true;
  }
}

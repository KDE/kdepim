/*******************************************************************************
**
** Filename   : util
** Created on : 03 April, 2005
** Copyright  : (c) 2005 Till Adam
** Email      : <adam@kde.org>
**
*******************************************************************************/

/*******************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   It is distributed in the hope that it will be useful, but
**   WITHOUT ANY WARRANTY; without even the implied warranty of
**   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**   General Public License for more details.
**
**   You should have received a copy of the GNU General Public License
**   along with this program; if not, write to the Free Software
**   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
**   In addition, as a special exception, the copyright holders give
**   permission to link the code of this program with any edition of
**   the Qt library by Trolltech AS, Norway (or with modified versions
**   of Qt that use the same license as Qt), and distribute linked
**   combinations including the two.  You must obey the GNU General
**   Public License in all respects for all of the code used other than
**   Qt.  If you modify this file, you may extend this exception to
**   your version of the file, but you are not obligated to do so.  If
**   you do not wish to do so, delete this exception statement from
**   your version.
**
*******************************************************************************/
#include "util.h"

#include "iconnamecache.h"
#include "nodehelper.h"

#include "messagecore/globalsettings.h"
#include "messagecore/nodehelper.h"
#include "messagecore/stringutil.h"

#include <KMime/Message>

#include <kcharsets.h>
#include <KFileDialog>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>
#include <kdebug.h>
#include <KMimeType>
#include <KTemporaryFile>

#include <QTextCodec>
#include <QWidget>

using namespace MessageViewer;

bool Util::checkOverwrite( const KUrl &url, QWidget *w )
{
  if ( KIO::NetAccess::exists( url, KIO::NetAccess::DestinationSide, w ) ) {
    if ( KMessageBox::Cancel == KMessageBox::warningContinueCancel(
         w,
         i18n( "A file named \"%1\" already exists. "
             "Are you sure you want to overwrite it?", url.prettyUrl() ),
             i18n( "Overwrite File?" ),
                   KStandardGuiItem::overwrite() ) )
      return false;
  }
  return true;
}

QString Util::fileNameForMimetype( const QString &mimeType, int iconSize,
                                   const QString &fallbackFileName1,
                                   const QString &fallbackFileName2 )
{
  QString fileName;
  KMimeType::Ptr mime = KMimeType::mimeType( mimeType, KMimeType::ResolveAliases );
  if ( mime ) {
    fileName = mime->iconName();
  } else {
    kWarning() << "unknown mimetype" << mimeType;
  }

  if ( fileName.isEmpty() )
  {
    fileName = fallbackFileName1;
    if ( fileName.isEmpty() )
      fileName = fallbackFileName2;
    if ( !fileName.isEmpty() ) {
      fileName = KMimeType::findByPath( "/tmp/" + fileName, 0, true )->iconName();
    }
  }

  return IconNameCache::instance()->iconPath( fileName, iconSize );
}

#ifdef Q_WS_MACX
#include <QDesktopServices>
#endif

bool Util::handleUrlOnMac( const KUrl& url )
{
#ifdef Q_WS_MACX
  QDesktopServices::openUrl( url );
  return true;
#else
  Q_UNUSED( url );
  return false;
#endif
}

QList<KMime::Content*> Util::allContents( const KMime::Content *message )
{
  KMime::Content::List result;
  KMime::Content *child = MessageCore::NodeHelper::firstChild( message );
  if ( child ) {
    result += child;
    result += allContents( child );
  }
  KMime::Content *next = MessageCore::NodeHelper::nextSibling( message );
  if ( next ) {
    result += next;
    result += allContents( next );
  }

  return result;
}

QList<KMime::Content*> Util::extractAttachments( const KMime::Message *message )
{
  KMime::Content::List contents = allContents( message );
  for ( KMime::Content::List::iterator it = contents.begin();
        it != contents.end(); ) {
    // only body parts which have a filename or a name parameter (except for
    // the root node for which name is set to the message's subject) are
    // considered attachments
    KMime::Content* content = *it;
    if ( content->contentDisposition()->filename().trimmed().isEmpty() &&
          ( content->contentType()->name().trimmed().isEmpty() ||
            content == message ) ) {
      KMime::Content::List::iterator delIt = it;
      ++it;
      contents.erase( delIt );
    } else {
      ++it;
    }
  }
  return contents;
}

bool Util::saveContents( QWidget *parent, const QList<KMime::Content*> &contents )
{
  KUrl url, dirUrl;
  if ( contents.count() > 1 ) {
    // get the dir
    dirUrl = KFileDialog::getExistingDirectoryUrl( KUrl( "kfiledialog:///saveAttachment" ),
                                                   parent,
                                                   i18n( "Save Attachments To" ) );
    if ( !dirUrl.isValid() ) {
      return false;
    }

    // we may not get a slash-terminated url out of KFileDialog
    dirUrl.adjustPath( KUrl::AddTrailingSlash );
  }
  else {
    // only one item, get the desired filename
    KMime::Content *content = contents.first();
    QString fileName = NodeHelper::fileName( content );
    fileName = MessageCore::StringUtil::cleanFileName( fileName );
    if ( fileName.isEmpty() ) {
      fileName = i18nc( "filename for an unnamed attachment", "attachment.1" );
    }
    url = KFileDialog::getSaveUrl( KUrl( "kfiledialog:///saveAttachment/" + fileName ),
                                   QString(),
                                   parent,
                                   i18n( "Save Attachment" ) );
    if ( url.isEmpty() ) {
      return false;
    }
  }

  QMap< QString, int > renameNumbering;

  bool globalResult = true;
  int unnamedAtmCount = 0;
  bool overwriteAll = false;
  foreach( KMime::Content *content, contents ) {
    KUrl curUrl;
    if ( !dirUrl.isEmpty() ) {
      curUrl = dirUrl;
      QString fileName = MessageViewer::NodeHelper::fileName( content );
      fileName = MessageCore::StringUtil::cleanFileName( fileName );
      if ( fileName.isEmpty() ) {
        ++unnamedAtmCount;
        fileName = i18nc( "filename for the %1-th unnamed attachment",
                          "attachment.%1", unnamedAtmCount );
      }
      curUrl.setFileName( fileName );
    } else {
      curUrl = url;
    }

    if ( !curUrl.isEmpty() ) {

      // Rename the file if we have already saved one with the same name:
      // try appending a number before extension (e.g. "pic.jpg" => "pic_2.jpg")
      QString origFile = curUrl.fileName();
      QString file = origFile;

      while ( renameNumbering.contains(file) ) {
        file = origFile;
        int num = renameNumbering[file] + 1;
        int dotIdx = file.lastIndexOf('.');
        file = file.insert( (dotIdx>=0) ? dotIdx : file.length(), QString("_") + QString::number(num) );
      }
      curUrl.setFileName(file);

      // Increment the counter for both the old and the new filename
      if ( !renameNumbering.contains(origFile))
        renameNumbering[origFile] = 1;
      else
        renameNumbering[origFile]++;

      if ( file != origFile ) {
        if ( !renameNumbering.contains(file))
          renameNumbering[file] = 1;
        else
          renameNumbering[file]++;
      }


      if ( !overwriteAll && KIO::NetAccess::exists( curUrl, KIO::NetAccess::DestinationSide, parent ) ) {
        if ( contents.count() == 1 ) {
          if ( KMessageBox::warningContinueCancel( parent,
                i18n( "A file named <br><filename>%1</filename><br>already exists.<br><br>Do you want to overwrite it?",
                  curUrl.fileName() ),
                i18n( "File Already Exists" ), KGuiItem(i18n("&Overwrite")) ) == KMessageBox::Cancel) {
            continue;
          }
        }
        else {
          int button = KMessageBox::warningYesNoCancel(
                parent,
                i18n( "A file named <br><filename>%1</filename><br>already exists.<br><br>Do you want to overwrite it?",
                  curUrl.fileName() ),
                i18n( "File Already Exists" ), KGuiItem(i18n("&Overwrite")),
                KGuiItem(i18n("Overwrite &All")) );
          if ( button == KMessageBox::Cancel )
            continue;
          else if ( button == KMessageBox::No )
            overwriteAll = true;
        }
      }
      // save
      const bool result = saveContent( parent, content, curUrl );
      if ( !result )
        globalResult = result;
    }
  }

  return globalResult;
}

bool Util::saveContent( QWidget *parent, KMime::Content* content, const KUrl& url )
{
  // FIXME: This is all horribly broken. First of all, creating a NodeHelper and then immediatley
  //        reading out the encryption/signature state will not work at all.
  //        Then, topLevel() will not work for attachments that are inside encrypted parts.
  //        What should actually be done is either passing in an ObjectTreeParser that has already
  //        parsed the message, or creating an OTP here (which would have the downside that the
  //        password dialog for decrypting messages is shown twice)
#if 0 // totally broken
  KMime::Content *topContent  = content->topLevel();
  MessageViewer::NodeHelper *mNodeHelper = new MessageViewer::NodeHelper;
  bool bSaveEncrypted = false;
  bool bEncryptedParts = mNodeHelper->encryptionState( content ) != MessageViewer::KMMsgNotEncrypted;
  if( bEncryptedParts )
    if( KMessageBox::questionYesNo( parent,
                                    i18n( "The part %1 of the message is encrypted. Do you want to keep the encryption when saving?",
                                          url.fileName() ),
                                    i18n( "KMail Question" ), KGuiItem(i18n("Keep Encryption")), KGuiItem(i18n("Do Not Keep")) ) ==
        KMessageBox::Yes )
      bSaveEncrypted = true;

  bool bSaveWithSig = true;
  if(mNodeHelper->signatureState( content ) != MessageViewer::KMMsgNotSigned )
    if( KMessageBox::questionYesNo( parent,
                                    i18n( "The part %1 of the message is signed. Do you want to keep the signature when saving?",
                                          url.fileName() ),
                                    i18n( "KMail Question" ), KGuiItem(i18n("Keep Signature")), KGuiItem(i18n("Do Not Keep")) ) !=
        KMessageBox::Yes )
      bSaveWithSig = false;

  QByteArray data;
  if( bSaveEncrypted || !bEncryptedParts) {
    KMime::Content *dataNode = content;
    QByteArray rawReplyString;
    bool gotRawReplyString = false;
    if ( !bSaveWithSig ) {
      if ( topContent->contentType()->mimeType() == "multipart/signed" )  {
        // carefully look for the part that is *not* the signature part:
        if ( ObjectTreeParser::findType( topContent, "application/pgp-signature", true, false ) ) {
          dataNode = ObjectTreeParser::findTypeNot( topContent, "application", "pgp-signature", true, false );
        } else if ( ObjectTreeParser::findType( topContent, "application/pkcs7-mime" , true, false ) ) {
          dataNode = ObjectTreeParser::findTypeNot( topContent, "application", "pkcs7-mime", true, false );
        } else {
          dataNode = ObjectTreeParser::findTypeNot( topContent, "multipart", "", true, false );
        }
      } else {
        EmptySource emptySource;
        ObjectTreeParser otp( &emptySource, 0, 0,false, false, false );

        // process this node and all it's siblings and descendants
        mNodeHelper->setNodeUnprocessed( dataNode, true );
        otp.parseObjectTree( dataNode );

        rawReplyString = otp.rawReplyString();
        gotRawReplyString = true;
      }
    }
    QByteArray cstr = gotRawReplyString
      ? rawReplyString
      : dataNode->decodedContent();
    data = KMime::CRLFtoLF( cstr );
  }
#else
  const QByteArray data = content->decodedContent();
  kWarning() << "Port the encryption/signature handling when saving a KMime::Content.";
#endif
  QDataStream ds;
  QFile file;
  KTemporaryFile tf;
  if ( url.isLocalFile() )
  {
    // save directly
    file.setFileName( url.toLocalFile() );
    if ( !file.open( QIODevice::WriteOnly ) )
    {
      KMessageBox::error( parent,
                          i18nc( "1 = file name, 2 = error string",
                                  "<qt>Could not write to the file<br><filename>%1</filename><br><br>%2",
                                  file.fileName(),
                                  file.errorString() ),
                          i18n( "Error saving attachment" ) );
      return false;
    }

    const int permissions = MessageViewer::Util::getWritePermissions();
    if ( permissions >= 0 )
      fchmod( file.handle(), permissions );

    ds.setDevice( &file );
  } else
  {
    // tmp file for upload
    tf.open();
    ds.setDevice( &tf );
  }

  if ( ds.writeRawData( data.data(), data.size() ) == -1)
    {
      QFile *f = static_cast<QFile *>( ds.device() );
      KMessageBox::error( parent,
                          i18nc( "1 = file name, 2 = error string",
                                 "<qt>Could not write to the file<br><filename>%1</filename><br><br>%2",
                                 f->fileName(),
                                 f->errorString() ),
                          i18n( "Error saving attachment" ) );
      return false;
    }

  if ( !url.isLocalFile() )
    {
      // QTemporaryFile::fileName() is only defined while the file is open
      QString tfName = tf.fileName();
      tf.close();
      if ( !KIO::NetAccess::upload( tfName, url, parent ) )
        {
          KMessageBox::error( parent,
                              i18nc( "1 = file name, 2 = error string",
                                     "<qt>Could not write to the file<br><filename>%1</filename><br><br>%2",
                                     url.prettyUrl(),
                                     KIO::NetAccess::lastErrorString() ),
                              i18n( "Error saving attachment" ) );
          return false;
        }
    }
  else
    file.close();

#if 0
  mNodeHelper->removeTempFiles();
  delete mNodeHelper;
#endif
  return true;
}


int Util::getWritePermissions()
{
  // #79685, #232001 by default use the umask the user defined, but let it be configurable
  if ( MessageCore::GlobalSettings::self()->disregardUmask() ) {
    return S_IRUSR | S_IWUSR;
  } else {
    return -1;
  }
}



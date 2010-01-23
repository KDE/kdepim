/*
 Copyright 2009 Olivier Trichet <nive@nivalis.org>

 Permission to use, copy, modify, and distribute this software
 and its documentation for any purpose and without fee is hereby
 granted, provided that the above copyright notice appear in all
 copies and that both that the copyright notice and this
 permission notice and warranty disclaimer appear in supporting
 documentation, and that the name of the author not be used in
 advertising or publicity pertaining to distribution of the
 software without specific, written prior permission.

 The author disclaim all warranties with regard to this
 software, including all implied warranties of merchantability
 and fitness.  In no event shall the author be liable for any
 special, indirect or consequential damages or any damages
 whatsoever resulting from loss of use, data or profits, whether
 in an action of contract, negligence or other tortious action,
 arising out of or in connection with the use or performance of
 this software.
*/

#include "startup.h"

#include "knaccountmanager.h"
#include "knglobals.h"
#include "knnntpaccount.h"

#include <KDebug>
#include <KIconLoader>
#include <KLocale>
#include <KPIMIdentities/Identity>
#include <KPIMIdentities/IdentityManager>
#include <KPIMIdentities/Signature>
#include <KStandardDirs>
#include <QDir>


namespace KNode {
namespace Utilities {

void Startup::loadLibrariesIconsAndTranslations() const
{
  KIconLoader::global()->addAppDir( "knode" );
  KIconLoader::global()->addAppDir( "libkdepim" );

  KGlobal::locale()->insertCatalog( "libkdepim" );
  KGlobal::locale()->insertCatalog( "libkpgp" );
}



void Startup::updateDataAndConfiguration() const
{
  KConfig *conf = KNGlobals::self()->config();
  KConfigGroup cg( conf, "GENERAL" );
  QList<int> updateIds = cg.readEntry( "KNode::Utilities::Startup::updateId", QList<int>() );
  if ( !updateIds.contains( 1 ) ) {
    convertPre45Identities();
    updateIds << 1;
  }
  cg.writeEntry( "KNode::Utilities::Startup::updateId", updateIds );
}

void Startup::convertPre45Identities() const
{
  KSharedConfigPtr conf;
  KConfigGroup cg;

  // Global identity
  kDebug() << "Converting global identity";
  cg = KConfigGroup( KNGlobals::self()->config(), "IDENTITY" );
  convertPre45Identity( cg );

  // Server accounts and groups identities
  QDir dir( KStandardDirs::locateLocal( "data", "knode/", false/*create*/ ) );
  QStringList serverPaths = dir.entryList( QStringList( "nntp.*" ), QDir::Dirs );
  foreach ( const QString &subPath, serverPaths ) {
    QDir serverDir( dir.absolutePath() + '/' + subPath );
    // server account
    QFile f( serverDir.absolutePath() + "/info" );
    kDebug() << "Reading" << f.fileName();
    if ( f.exists() ) {
      conf = KSharedConfig::openConfig( f.fileName(), KConfig::SimpleConfig );
      cg = KConfigGroup( conf, QString() );
      kDebug() << "Converting identity of account from" << f.fileName() << ':' << cg.readEntry( "server" );
      convertPre45Identity( cg );

      // groups
      QStringList grpInfos = serverDir.entryList( QStringList( "*.grpinfo" ), QDir::Files );
      foreach ( const QString &grpInfo, grpInfos ) {
        QFile infoFile( serverDir.absolutePath() + '/' + grpInfo );
        if ( infoFile.exists() ) {
          conf = KSharedConfig::openConfig( infoFile.fileName(), KConfig::SimpleConfig );
          cg = KConfigGroup( conf, QString() );
          kDebug() << "Converting identity of group from" << infoFile.fileName();
          convertPre45Identity( cg );
        }
      }
    }
  }
}

void Startup::convertPre45Identity( KConfigGroup &cg ) const
{
  KPIMIdentities::IdentityManager* im = KNGlobals::self()->identityManager();

  // Load the identity from cg
  KPIMIdentities::Identity identity;
  identity.setFullName( cg.readEntry( "Name" ).trimmed() );
  identity.setEmailAddr( cg.readEntry( "Email" ).trimmed() );
  identity.setOrganization( cg.readEntry( "Org" ).trimmed() );
  identity.setReplyToAddr( cg.readEntry( "Reply-To" ).trimmed() );
  identity.setProperty( "Mail-Copies-To", cg.readEntry( "Mail-Copies-To" ).trimmed() );
  identity.setPGPSigningKey( cg.readEntry( "SigningKey" ).toLatin1() );

  KPIMIdentities::Signature signature;
  if ( cg.readEntry( "UseSigFile", false ) ) {
    if ( !cg.readEntry( "sigFile" ).trimmed().isEmpty() ) {
      if ( cg.readEntry( "UseSigGenerator", false ) ) {
        // output of a command execution
        signature.setUrl( cg.readEntry( "sigFile" ), true/*isExecutable*/ );
      } else {
        // content of a file
        signature.setUrl( cg.readEntry( "sigFile" ), false/*isExecutable*/ );
      }
    }
  } else if ( !cg.readEntry( "sigText" ).trimmed().isEmpty() ) {
    signature.setText( cg.readEntry( "sigText" ) );
  }
  identity.setSignature( signature );

  // Save to the new backend
  if ( // Is identity empty ?
         !identity.fullName().isEmpty()
      || !identity.emailAddr().isEmpty()
      || !identity.organization().isEmpty()
      || !identity.replyToAddr().isEmpty()
      || !identity.property( "Mail-Copies-To" ).toString().isEmpty()
      || !identity.pgpSigningKey().isEmpty()
      || identity.signature().type() != KPIMIdentities::Signature::Disabled
   ) {
    // If an identity (even created by another application) has the
    // same information as currently store in KNode, we reused it.
    bool isDuplicate = false;
    KPIMIdentities::IdentityManager::ConstIterator it = im->begin();
    while ( it != im->end() ) {
      KPIMIdentities::Identity otherId = (*it);
      isDuplicate = (
                      identity.fullName() == otherId.fullName()
                   && identity.emailAddr() == otherId.emailAddr()
                   && identity.organization() == otherId.organization()
                   && identity.replyToAddr() == otherId.replyToAddr()
                   && identity.property( "Mail-Copies-To" ).toString() == otherId.property( "Mail-Copies-To" ).toString()
                   && identity.pgpSigningKey() == otherId.pgpSigningKey()
                   && identity.signature() == otherId.signature()
                );
      if ( isDuplicate ) {
        break;
      }
      ++it;
    }

    if ( isDuplicate ) {
      identity = (*it);
      kDebug() << "An identity containing the same information was found : " << identity.identityName();
    } else {
      identity = im->newFromExisting( identity, im->makeUnique( identity.fullName() ) );
      // Ensure that this new identity is seen by the Iterator above in the next call of this method.
      im->commit();
      kDebug() << "Adding a new identity named " << identity.identityName();
    }
    cg.writeEntry( "identity", identity.uoid() );
  } else {
    kDebug() << "Empty identity found";
  }

  // Delete old entry
  // TODO: readd this
//   cg.deleteEntry( "Name" );
//   cg.deleteEntry( "Email" );
//   cg.deleteEntry( "Org" );
//   cg.deleteEntry( "Reply-To" );
//   cg.deleteEntry( "Mail-Copies-To" );
//   cg.deleteEntry( "SigningKey" );
//   cg.deleteEntry( "UseSigFile" );
//   cg.deleteEntry( "sigFile" );
//   cg.deleteEntry( "UseSigGenerator" );
//   cg.deleteEntry( "sigText" );
}

} // namespace Utilities
} // namespace KNode

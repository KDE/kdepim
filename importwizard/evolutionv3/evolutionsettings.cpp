/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "evolutionsettings.h"

#include <kpimidentities/identity.h>

#include <mailtransport/transportmanager.h>

#include <KConfig>
#include <KConfigGroup>
#include <KDebug>

#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QDebug>

EvolutionSettings::EvolutionSettings( const QString& filename, ImportWizard *parent )
    :AbstractSettings( parent )
{
  //Read gconf file
  QFile file(filename);
  if ( !file.open( QIODevice::ReadOnly ) ) {
    qDebug()<<" We can't open file"<<filename;
    return;
  }

  QDomDocument doc;
  QString errorMsg;
  int errorRow;
  int errorCol;
  if ( !doc.setContent( &file, &errorMsg, &errorRow, &errorCol ) ) {
    kDebug() << "Unable to load document.Parse error in line " << errorRow
             << ", col " << errorCol << ": " << errorMsg;
    return;
  }
  QDomElement config = doc.documentElement();

  if ( config.isNull() ) {
    kDebug() << "No config found";
    return;
  }
  for ( QDomElement e = config.firstChildElement(); !e.isNull(); e = e.nextSiblingElement() ) {
    const QString tag = e.tagName();
    if ( tag == QLatin1String( "entry" ) ) {
      if ( e.hasAttribute( "name" ) ) {
        const QString attr = e.attribute("name");
        if ( attr == QLatin1String( "accounts" ) ) {
          readAccount(e);
        } else if ( attr == QLatin1String( "signatures" ) ) {
          readSignatures( e );
        }
      }  
    }
  }
}

EvolutionSettings::~EvolutionSettings()
{
}

void EvolutionSettings::readSignatures(const QDomElement &account)
{
  for ( QDomElement signatureConfig = account.firstChildElement(); !signatureConfig.isNull(); signatureConfig = signatureConfig.nextSiblingElement() ) {
    if(signatureConfig.tagName() == QLatin1String("li")) {
      QDomElement stringValue = signatureConfig.firstChildElement();
      extractSignatureInfo(stringValue.text());
    }
  }
}

void EvolutionSettings::extractSignatureInfo( const QString&info )
{
  qDebug()<<" signature info "<<info;
  //Read QDomElement
  QDomDocument signature;
  QString errorMsg;
  int errorRow;
  int errorCol;
  if ( !signature.setContent( info, &errorMsg, &errorRow, &errorCol ) ) {
    kDebug() << "Unable to load document.Parse error in line " << errorRow
             << ", col " << errorCol << ": " << errorMsg;
    return;
  }

  QDomElement domElement = signature.documentElement();

  if ( domElement.isNull() ) {
    kDebug() << "Signature not found";
    return;
  }
  for ( QDomElement e = domElement.firstChildElement(); !e.isNull(); e = e.nextSiblingElement() ) {
    KPIMIdentities::Signature signature;
    
    const QString tag = e.tagName();
    const QString uid = e.attribute( QLatin1String( "uid" ) );
    const QString signatureName = e.attribute( QLatin1String( "name" ) );
    const QString format = e.attribute( QLatin1String( "text" ) );
    const bool automatic = ( e.attribute( QLatin1String( "auto" ) ) == QLatin1String( "true" ) );
    
    if ( tag == QLatin1String( "filename" ) ) {
      //TODO store it
    }
    
    if ( automatic )
      signature.setType( KPIMIdentities::Signature::FromCommand );
    else
      signature.setType( KPIMIdentities::Signature::FromFile );
    //void setUrl( const QString &url, bool isExecutable=false );
    mMapSignature.insert( uid, signature );
        
    qDebug()<<" signature tag :"<<tag;
  }

//<signature name="html" uid="1332775655.21659.4@krita" auto="false" format="text/html"><filename>signature-1</filename></signature>
  //TODO signature path :  ~/.local/share/evolution/signatures/*
}

void EvolutionSettings::readAccount(const QDomElement &account)
{
  for ( QDomElement accountConfig = account.firstChildElement(); !accountConfig.isNull(); accountConfig = accountConfig.nextSiblingElement() ) {
    if(accountConfig.tagName() == QLatin1String("li")) {
      QDomElement stringValue = accountConfig.firstChildElement();
      extractAccountInfo(stringValue.text());
    }
  }
}

void EvolutionSettings::extractAccountInfo(const QString& info)
{
  qDebug()<<" info "<<info;
  //Read QDomElement
  QDomDocument account;
  QString errorMsg;
  int errorRow;
  int errorCol;
  if ( !account.setContent( info, &errorMsg, &errorRow, &errorCol ) ) {
    kDebug() << "Unable to load document.Parse error in line " << errorRow
             << ", col " << errorCol << ": " << errorMsg;
    return;
  }

  QDomElement domElement = account.documentElement();

  if ( domElement.isNull() ) {
    kDebug() << "Account not found";
    return;
  }
  KPIMIdentities::Identity* newIdentity = createIdentity();
  for ( QDomElement e = domElement.firstChildElement(); !e.isNull(); e = e.nextSiblingElement() ) {
    const QString tag = e.tagName();
    qDebug()<<" tag :"<<tag;
    if ( tag == QLatin1String( "identity" ) )
    {
      for ( QDomElement identity = e.firstChildElement(); !identity.isNull(); identity = identity.nextSiblingElement() ) {
        const QString identityTag = identity.tagName();
        if ( identityTag == QLatin1String( "name" ) )
        {
          newIdentity->setIdentityName( identity.text() );
        }
        else if ( identityTag == QLatin1String( "addr-spec" ) )
        {
          newIdentity->setPrimaryEmailAddress(identity.text());
        }
        else if ( identityTag == QLatin1String( "organization" ) )
        {
          newIdentity->setOrganization(identity.text());
        }
        else if ( identityTag == QLatin1String( "signature" ) )
        {
          if ( identity.hasAttribute( "uid" ) ) {
            newIdentity->setSignature( mMapSignature.value( identity.attribute( "uid" ) ) );
          }
        }
        else if ( identityTag == QLatin1String( "reply-to" ) )
        {
          newIdentity->setReplyToAddr( identity.text() );
        }
        else
        {
          qDebug()<<" tag identity not found :"<<identityTag;
        }
      }
    }
    else if ( tag == QLatin1String( "source" ) )
    {
      //TODO imap ? pop3 ? 
    }
    else if ( tag == QLatin1String( "transport" ) )
    {
      if ( e.hasAttribute( "save-passwd" ) && e.attribute( "save-passwd" ) == QLatin1String( "true" ) )
      {
        //TODO save to kwallet ?
      }
      
      MailTransport::Transport *transport = createTransport();
      for ( QDomElement smtp = e.firstChildElement(); !smtp.isNull(); smtp = smtp.nextSiblingElement() ) {
        const QString smtpTag = smtp.tagName();
        if ( smtpTag == QLatin1String( "url" ) ) {
          qDebug()<<" smtp.text() :"<<smtp.text();
          QUrl smtpUrl( smtp.text() );

          transport->setHost( smtpUrl.host() );
          transport->setName( smtpUrl.host() );

          const int port = smtpUrl.port();
          if ( port > 0 )
            transport->setPort( port );
          
        } else {
          qDebug()<<" smtp tag unknow :"<<smtpTag;
        }
      }
      //TODO authentification
      transport->writeConfig();
      MailTransport::TransportManager::self()->addTransport( transport );
      MailTransport::TransportManager::self()->setDefaultTransport( transport->id() );
    }
    else if ( tag == QLatin1String( "drafts-folder" ) )
    {
      const QString selectedFolder = adaptFolder( e.text().remove( QLatin1String( "folder://" ) ) );
      newIdentity->setDrafts(selectedFolder); 
    }
    else if ( tag == QLatin1String( "sent-folder" ) )
    {
      const QString selectedFolder = adaptFolder( e.text().remove( QLatin1String( "folder://" ) ) );
      newIdentity->setFcc(selectedFolder);
    }
    else if ( tag == QLatin1String( "auto-cc" ) )
    {
      if ( e.hasAttribute( "always" ) && ( e.attribute( "always" ) == QLatin1String( "true" ) ) )
      {
        QDomElement recipient = e.firstChildElement();
        const QString text = recipient.text();
        newIdentity->setReplyToAddr(text);
      }
    }
    else if ( tag == QLatin1String( "auto-bcc" ) )
    {
      if ( e.hasAttribute( "always" ) && ( e.attribute( "always" ) == QLatin1String( "true" ) ) )
      {
        QDomElement recipient = e.firstChildElement();
        const QString text = recipient.text();
        newIdentity->setBcc(text);
      }
    }
    else if ( tag == QLatin1String( "receipt-policy" ) )
    {
      //TODO
    }
    else if ( tag == QLatin1String( "pgp" ) )
    {
      //TODO
    }
    else if ( tag == QLatin1String( "smime" ) )
    {
      //TODO
    }
    else
      qDebug()<<" tag not know :"<<tag;

  }
  storeIdentity(newIdentity);
}

/*
  This file is part of KAddressBook.

  Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#include "grantleecontactgroupformatter.h"

#include <grantlee/context.h>
#include <grantlee/engine.h>
#include <grantlee/templateloader.h>

#include <akonadi/contact/contactgroupexpandjob.h>
#include <akonadi/item.h>
#include <kabc/addressee.h>
#include <kabc/contactgroup.h>
#include <kcolorscheme.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstringhandler.h>

using namespace Akonadi;

class GrantleeContactGroupFormatter::Private
{
  public:
    Private( const QString &templatePath )
    {
      mEngine = new Grantlee::Engine;

      mTemplateLoader = Grantlee::FileSystemTemplateLoader::Ptr( new Grantlee::FileSystemTemplateLoader );
      mTemplateLoader->setTemplateDirs( QStringList() << templatePath );
      mTemplateLoader->setTheme( QLatin1String( "default" ) );

      mEngine->addTemplateLoader( mTemplateLoader );
      mSelfcontainedTemplate = mEngine->loadByName( "contactgroup.html" );
      if ( mSelfcontainedTemplate->error() )
        mErrorMessage += mSelfcontainedTemplate->errorString();

      mEmbeddableTemplate = mEngine->loadByName( "contactgroup_embedded.html" );
      if ( mEmbeddableTemplate->error() )
        mErrorMessage += mEmbeddableTemplate->errorString();
    }

    ~Private()
    {
      delete mEngine;
    }

    QVector<QObject*> mObjects;
    Grantlee::Engine *mEngine;
    Grantlee::FileSystemTemplateLoader::Ptr mTemplateLoader;
    Grantlee::Template mSelfcontainedTemplate;
    Grantlee::Template mEmbeddableTemplate;
    QString mErrorMessage;
};

GrantleeContactGroupFormatter::GrantleeContactGroupFormatter( const QString &templatePath )
  : d( new Private( templatePath ) )
{
}

GrantleeContactGroupFormatter::~GrantleeContactGroupFormatter()
{
  delete d;
}

inline static void setHashField( QVariantHash &hash, const QString &name, const QString &value )
{
  if ( !value.isEmpty() )
    hash.insert( name, value );
}

static QVariantHash memberHash( const KABC::ContactGroup::Data &data )
{
  QVariantHash memberObject;

  setHashField( memberObject, QLatin1String( "name" ), data.name() );
  setHashField( memberObject, QLatin1String( "email" ), data.email() );

  KABC::Addressee contact;
  contact.setFormattedName( data.name() );
  contact.insertEmail( data.email() );

  const QString emailLink = QLatin1String( "<a href=\"mailto:" ) +
                            QString::fromLatin1( KUrl::toPercentEncoding( contact.fullEmail() ) ) +
                            QString::fromLatin1( "\">%1</a>" ).arg( contact.preferredEmail() );

  setHashField( memberObject, QLatin1String( "emailLink" ), emailLink );

  return memberObject;
}

QString GrantleeContactGroupFormatter::toHtml( HtmlForm form ) const
{
  if ( !d->mErrorMessage.isEmpty() )
    return d->mErrorMessage;

  KABC::ContactGroup group;
  const Akonadi::Item localItem = item();
  if ( localItem.isValid() && localItem.hasPayload<KABC::ContactGroup>() )
    group = localItem.payload<KABC::ContactGroup>();
  else
    group = contactGroup();

  if ( group.name().isEmpty() && group.count() == 0 ) // empty group
    return QString();

  if ( group.contactReferenceCount() != 0 ) {
    // we got a contact group with unresolved references -> we have to resolve it ourself
    // this shouldn't be the normal case, actually the calling code should pass in an already resolved
    // contact group
    ContactGroupExpandJob *job = new ContactGroupExpandJob( group );
    if ( job->exec() ) {
      group.removeAllContactData();
      foreach ( const KABC::Addressee &contact, job->contacts() ) {
        group.append( KABC::ContactGroup::Data( contact.realName(), contact.preferredEmail() ) );
      }
    }
  }

  QVariantHash contactGroupObject;

  setHashField( contactGroupObject, QLatin1String( "name" ), group.name() );

  // Group members
  QVariantList members;
  for ( uint i = 0; i < group.dataCount(); ++i ) {
    members << memberHash( group.data( i ) );
  }

  contactGroupObject.insert( QLatin1String( "members" ), members );

  // Additional fields
  QVariantList fields;
  foreach ( const QVariantMap &field, additionalFields() ) {
    QVariantHash fieldObject;
    setHashField( fieldObject, QLatin1String( "key" ), field.value( QLatin1String( "key" ) ).toString() );
    setHashField( fieldObject, QLatin1String( "title" ), field.value( QLatin1String( "title" ) ).toString() );
    setHashField( fieldObject, QLatin1String( "value" ), field.value( QLatin1String( "value" ) ).toString() );

    fields << fieldObject;
  }

  contactGroupObject.insert( QLatin1String( "additionalFields" ), fields );

  QVariantHash colorsObject;
  colorsObject.insert( "linkColor", KColorScheme( QPalette::Active, KColorScheme::View ).foreground().color().name() );
  colorsObject.insert( "textColor", KColorScheme( QPalette::Active, KColorScheme::View ).foreground().color().name() );
  colorsObject.insert( "backgroundColor", KColorScheme( QPalette::Active, KColorScheme::View ).background().color().name() );

  QVariantHash mapping;
  mapping.insert( "contactGroup", contactGroupObject );
  mapping.insert( "colors", colorsObject );

  Grantlee::Context context( mapping );

  if ( form == SelfcontainedForm )
    return d->mSelfcontainedTemplate->render( &context );
  else if ( form == EmbeddableForm )
    return d->mEmbeddableTemplate->render( &context );
  else
    return QString();
}

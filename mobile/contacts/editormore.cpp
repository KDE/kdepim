/*
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

#include "editormore.h"

#include "contactmetadata_p.h"
#include "customfieldeditordialog.h"
#include "customfieldeditwidget.h"
#include "customfieldmanager_p.h"
#include "settings.h"
#include "ui_editormore.h"
#include "ui_editormore_categoriespage.h"
#include "ui_editormore_customfieldspage.h"
#include "ui_editormore_namepage.h"
#include "ui_editormore_internetpage.h"
#include "ui_editormore_personalpage.h"

#include <calendarsupport/categoryconfig.h>
#include <incidenceeditor-ng/categorydialog.h>

#include <KABC/Addressee>

#ifndef Q_OS_WINCE
#include <phonon/mediaobject.h>
#endif

#include <QtCore/QBuffer>
#include <QtCore/QSignalMapper>
#include <QtCore/QUuid>
#include <QLabel>

class EditorMore::Private
{
  EditorMore *const q;

  public:
    explicit Private( EditorMore *parent ) : q( parent )
    {
      mUi.setupUi( parent );

      mMapper = new QSignalMapper( q );
      mMapper->setMapping( mUi.namePageButton, 0 );
      mMapper->setMapping( mUi.internetPageButton, 1 );
      mMapper->setMapping( mUi.personalPageButton, 2 );
      mMapper->setMapping( mUi.customFieldsPageButton, 3 );
      mMapper->setMapping( mUi.categoriesPageButton, 4 );

      // tokoe: enable when ContactMetaData is part of public API
      mUi.customFieldsPageButton->hide();


      QWidget *namePage = new QWidget;
      mNamePage.setupUi( namePage );
      mNamePage.pronunciationTitle->hide(); // not editable, so don't confuse the user
      mNamePage.pronunciationLabel->hide();

      QWidget *internetPage = new QWidget;
      mInternetPage.setupUi( internetPage );

      QWidget *personalPage = new QWidget;
      mPersonalPage.setupUi( personalPage );

      QWidget *customFieldsPage = new QWidget;
      mCustomFieldsPage.setupUi( customFieldsPage );

      QWidget *categoriesPage = new QWidget;
      mCategoriesPage.setupUi( categoriesPage );

      mUi.pageWidget->insertWidget( 0, namePage );
      mUi.pageWidget->insertWidget( 1, internetPage );
      mUi.pageWidget->insertWidget( 2, personalPage );
      mUi.pageWidget->insertWidget( 3, customFieldsPage );
      mUi.pageWidget->insertWidget( 4, categoriesPage );

      connect( mUi.namePageButton, SIGNAL(clicked()),
               mMapper, SLOT(map()) );
      connect( mUi.internetPageButton, SIGNAL(clicked()),
               mMapper, SLOT(map()) );
      connect( mUi.personalPageButton, SIGNAL(clicked()),
               mMapper, SLOT(map()) );
      connect( mUi.customFieldsPageButton, SIGNAL(clicked()),
               mMapper, SLOT(map()) );
      connect( mUi.categoriesPageButton, SIGNAL(clicked()),
               mMapper, SLOT(map()) );
      connect( mMapper, SIGNAL(mapped(int)),
               mUi.pageWidget, SLOT(setCurrentIndex(int)) );

      mUi.pageWidget->setCurrentIndex( 0 );

      connect( mNamePage.namePartsWidget, SIGNAL(nameChanged(KABC::Addressee)),
               mNamePage.displayNameWidget, SLOT(changeName(KABC::Addressee)) );
      connect( mNamePage.namePartsWidget, SIGNAL(nameChanged(KABC::Addressee)),
               q, SIGNAL(nameChanged(KABC::Addressee)) );
      connect( mNamePage.pronunciationLabel, SIGNAL(linkActivated(QString)),
               q, SLOT(playPronunciation()) );

      connect( mCustomFieldsPage.addCustomFieldButton, SIGNAL(clicked()),
               q, SLOT(addCustomField()) );

      connect( mCategoriesPage.categoriesButton, SIGNAL(clicked()),
               q, SLOT(configureCategories()) );

      mPersonalPage.birthdayDateEdit->setDate( QDate() );
      mPersonalPage.anniversaryDateEdit->setDate( QDate() );
    }

    void configureCategories()
    {
      CalendarSupport::CategoryConfig config( Settings::self(), 0 );

      IncidenceEditorNG::CategoryDialog dlg( &config, 0 );
      dlg.setCategoryList( mCategories );
      dlg.setSelected( mCategories );
      if ( dlg.exec() ) {
        mCategories = dlg.selectedCategories();
        mCategoriesPage.categoriesEdit->setText( mCategories.join( QLatin1String(", ") ) );
      }
    }

    void playPronunciation()
    {
      if ( mContact.sound().data().isEmpty() )
        return;

    // No phonon on WinCE (yet)
#ifndef Q_OS_WINCE
      Phonon::MediaObject* player = Phonon::createPlayer( Phonon::NotificationCategory );
      QBuffer* soundData = new QBuffer( player );
      soundData->setData( mContact.sound().data() );
      player->setCurrentSource( soundData );
      player->setParent( q );
      connect( player, SIGNAL(finished()), player, SLOT(deleteLater()) );
      player->play();
#endif
    }

    void addCustomField()
    {
      CustomField field;

      // We use a Uuid as default key, so we won't have any duplicated keys,
      // the user can still change it to something else in the editor dialog.
      // Since the key only allows [A-Za-z0-9\-]*, we have to remove the curly
      // braces as well.
      QString key = QUuid::createUuid().toString();
      key.remove( QLatin1Char( '{' ) );
      key.remove( QLatin1Char( '}' ) );

      field.setKey( key );

      CustomFieldEditorDialog dlg;
      dlg.setCustomField( field );

      if ( dlg.exec() ) {
        CustomFieldEditWidget *widget = new CustomFieldEditWidget;
        widget->setCustomField( dlg.customField() );

        mCustomFieldsPage.customFieldsLister->addWidget( widget );
      }
    }

  public:
    Ui::EditorMore mUi;
    Ui::NamePage mNamePage;
    Ui::InternetPage mInternetPage;
    Ui::PersonalPage mPersonalPage;
    Ui::CustomFieldsPage mCustomFieldsPage;
    Ui::CategoriesPage mCategoriesPage;
    QSignalMapper *mMapper;

    KABC::Addressee mContact;
    CustomField::List mLocalCustomFields;
    QStringList mCategories;
};

static QString loadCustom( const KABC::Addressee &contact, const QString &key )
{
  return contact.custom( QLatin1String( "KADDRESSBOOK" ), key );
}

static void storeCustom( KABC::Addressee &contact, const QString &key, const QString &value )
{
  if ( value.isEmpty() )
    contact.removeCustom( QLatin1String( "KADDRESSBOOK" ), key );
  else
    contact.insertCustom( QLatin1String( "KADDRESSBOOK" ), key, value );
}

static void splitCustomField( const QString &str, QString &app, QString &name, QString &value )
{
  const int colon = str.indexOf( QLatin1Char( ':' ) );
  if ( colon != -1 ) {
    const QString tmp = str.left( colon );
    value = str.mid( colon + 1 );

    const int dash = tmp.indexOf( QLatin1Char( '-' ) );
    if ( dash != -1 ) {
      app = tmp.left( dash );
      name = tmp.mid( dash + 1 );
    }
  }
}


EditorMore::EditorMore( QWidget *parent )
  : EditorBase( parent ), d( new Private( this ) )
{
}

EditorMore::~EditorMore()
{
  delete d;
}

void EditorMore::loadContact( const KABC::Addressee &contact, const Akonadi::ContactMetaData &metaData )
{
  Q_UNUSED( metaData );

  d->mContact = contact;

  // name page
  d->mNamePage.nicknameLineEdit->setText( contact.nickName() );
  d->mNamePage.namePartsWidget->loadContact( contact );
  d->mNamePage.displayNameWidget->loadContact( contact );
  // tokoe: enable when ContactMetaData is part of public API
  // d->mNamePage.displayNameWidget->setDisplayType( (DisplayNameEditWidget::DisplayType)metaData.displayNameMode() );
  const bool hasSound = !contact.sound().isEmpty();
  d->mNamePage.pronunciationTitle->setVisible( hasSound );
  d->mNamePage.pronunciationLabel->setVisible( hasSound );

  // internet page
  d->mInternetPage.urlLineEdit->setText( contact.url().url() );
  d->mInternetPage.blogLineEdit->setText( loadCustom( contact, QLatin1String( "BlogFeed" ) ) );
  d->mInternetPage.messagingLineEdit->setText( loadCustom( contact, QLatin1String( "X-IMAddress" ) ) );

  // personal page
  d->mPersonalPage.birthdayDateEdit->setDate( contact.birthday().date() );
  const QDate anniversary = QDate::fromString( loadCustom( contact, QLatin1String( "X-Anniversary" ) ), Qt::ISODate );
  d->mPersonalPage.anniversaryDateEdit->setDate( anniversary );
  d->mPersonalPage.partnerLineEdit->setText( loadCustom( contact, QLatin1String( "X-SpousesName" ) ) );

  // tokoe: enable when ContactMetaData is part of public API
  // loadCustomFields( contact, metaData );

  // categories page
  d->mCategories = contact.categories();
  d->mCategoriesPage.categoriesEdit->setText( d->mCategories.join( QLatin1String(", ") ) );
}

void EditorMore::loadCustomFields( const KABC::Addressee &contact, const Akonadi::ContactMetaData &metaData )
{
  d->mLocalCustomFields.clear();
  foreach ( const QVariant &description, metaData.customFieldDescriptions() )
    d->mLocalCustomFields.append( CustomField::fromVariantMap( description.toMap(), CustomField::LocalScope ) );

  CustomField::List externalCustomFields;

  CustomField::List globalCustomFields = CustomFieldManager::globalCustomFieldDescriptions();

  const QStringList customs = contact.customs();
  foreach ( const QString &custom, customs ) {

    QString app, name, value;
    splitCustomField( custom, app, name, value );

    // skip all well-known fields that have separated editor widgets
    if ( custom.startsWith( QLatin1String( "messaging/" ) ) ) // IM addresses
      continue;

    if ( app == QLatin1String( "KADDRESSBOOK" ) ) {
      static QSet<QString> blacklist;
      if ( blacklist.isEmpty() ) {
        blacklist << QLatin1String( "BlogFeed" )
                  << QLatin1String( "X-IMAddress" )
                  << QLatin1String( "X-Profession" )
                  << QLatin1String( "X-Office" )
                  << QLatin1String( "X-ManagersName" )
                  << QLatin1String( "X-AssistantsName" )
                  << QLatin1String( "X-Anniversary" )
                  << QLatin1String( "X-SpousesName" )
                  << QLatin1String( "X-Profession" );
      }

      if ( blacklist.contains( name ) ) // several KAddressBook specific fields
        continue;
    }

    // check whether it correspond to a local custom field
    bool isLocalCustomField = false;
    for ( int i = 0; i < d->mLocalCustomFields.count(); ++i ) {
      if ( d->mLocalCustomFields[ i ].key() == name ) {
        d->mLocalCustomFields[ i ].setValue( value );
        isLocalCustomField = true;
        break;
      }
    }

    // check whether it correspond to a global custom field
    bool isGlobalCustomField = false;
    for ( int i = 0; i < globalCustomFields.count(); ++i ) {
      if ( globalCustomFields[ i ].key() == name ) {
        globalCustomFields[ i ].setValue( value );
        isGlobalCustomField = true;
        break;
      }
    }

    // if not local and not global it must be external
    if ( !isLocalCustomField && !isGlobalCustomField ) {
      if ( app == QLatin1String( "KADDRESSBOOK" ) ) {
        // however if it starts with our prefix it might be that this is an outdated
        // global custom field, in this case treat it as local field of type text
        CustomField customField( name, name, CustomField::TextType, CustomField::LocalScope );
        customField.setValue( value );

        d->mLocalCustomFields << customField;
      } else {
        // it is really an external custom field
        const QString key = app + QLatin1Char( '-' ) + name;
        CustomField customField( key, key, CustomField::TextType, CustomField::ExternalScope );
        customField.setValue( value );

        externalCustomFields << customField;
      }
    }
  }

  const CustomField::List allCustomFields = CustomField::List() << d->mLocalCustomFields << globalCustomFields << externalCustomFields;
  foreach ( const CustomField &customField, allCustomFields ) {
    CustomFieldEditWidget *widget = new CustomFieldEditWidget;
    widget->setCustomField( customField );
    d->mCustomFieldsPage.customFieldsLister->addWidget( widget );
  }
}

void EditorMore::saveContact( KABC::Addressee &contact, Akonadi::ContactMetaData &metaData ) const
{
  Q_UNUSED( metaData );

  // name page
  contact.setNickName( d->mNamePage.nicknameLineEdit->text() );
  d->mNamePage.namePartsWidget->storeContact( contact );
  d->mNamePage.displayNameWidget->storeContact( contact );
  // tokoe: enable when ContactMetaData is part of public API
  // metaData.setDisplayNameMode( d->mNamePage.displayNameWidget->displayType() );

  // internet page
  contact.setUrl( d->mInternetPage.urlLineEdit->text() );
  storeCustom( contact, QLatin1String( "BlogFeed" ), d->mInternetPage.blogLineEdit->text() );
  storeCustom( contact, QLatin1String( "X-IMAddress" ), d->mInternetPage.messagingLineEdit->text() );

  // personal page
  storeCustom( contact, QLatin1String( "X-SpousesName" ), d->mPersonalPage.partnerLineEdit->text() );
  contact.setBirthday( QDateTime( d->mPersonalPage.birthdayDateEdit->date(), QTime(), contact.birthday().timeSpec() ) );
  const QString anniversary = d->mPersonalPage.anniversaryDateEdit->date().toString( Qt::ISODate );
  storeCustom( contact, QLatin1String( "X-Anniversary" ), anniversary );

  // tokoe: enable when ContactMetaData is part of public API
  // saveCustomFields( contact, metaData );

  // categories page
  contact.setCategories( d->mCategories );
}

void EditorMore::saveCustomFields( KABC::Addressee &contact, Akonadi::ContactMetaData &metaData ) const
{
  CustomField::List customFields;
  for ( int index = 0; index < d->mCustomFieldsPage.customFieldsLister->count(); ++index ) {
    CustomFieldEditWidget *widget = qobject_cast<CustomFieldEditWidget*>( d->mCustomFieldsPage.customFieldsLister->widget( index ) );
    if ( widget )
      customFields.append( widget->customField() );
  }

  foreach ( const CustomField &customField, customFields ) {
    // write back values for local and global scope, leave external untouched
    if ( customField.scope() != CustomField::ExternalScope ) {
      if ( !customField.value().isEmpty() )
        contact.insertCustom( QLatin1String( "KADDRESSBOOK" ), customField.key(), customField.value() );
      else
        contact.removeCustom( QLatin1String( "KADDRESSBOOK" ), customField.key() );
    }
  }

  // Now remove all fields that were available in loadContact (these are stored in mLocalCustomFields)
  // but are not part of customFields now, which means they have been removed or renamed by the user
  // in the editor dialog.
  foreach ( const CustomField &oldCustomField, d->mLocalCustomFields ) {
    if ( oldCustomField.scope() != CustomField::ExternalScope ) {

      bool fieldStillExists = false;
      foreach ( const CustomField &newCustomField, customFields ) {
        if ( newCustomField.scope() != CustomField::ExternalScope ) {
          if ( newCustomField.key() == oldCustomField.key() ) {
            fieldStillExists = true;
            break;
          }
        }
      }

      if ( !fieldStillExists )
        contact.removeCustom( QLatin1String( "KADDRESSBOOK" ), oldCustomField.key() );
    }
  }

  // And store the global custom fields descriptions as well
  CustomField::List globalCustomFields;
  foreach ( const CustomField &customField, customFields ) {
    if ( customField.scope() == CustomField::GlobalScope ) {
      globalCustomFields << customField;
    }
  }

  // store global custom fields
  CustomFieldManager::setGlobalCustomFieldDescriptions( globalCustomFields );

  // store local custom fields
  QVariantList descriptions;
  foreach ( const CustomField &field, customFields ) {
    if ( field.scope() == CustomField::LocalScope )
      descriptions.append( field.toVariantMap() );
  }

  metaData.setCustomFieldDescriptions( descriptions );
}

void EditorMore::updateOrganization( const QString &organization )
{
  d->mNamePage.displayNameWidget->changeOrganization( organization );
}

void EditorMore::updateName( const KABC::Addressee &contact )
{
  // this slot is called when the name has been changed in the 'General' page
  blockSignals( true );
  d->mNamePage.namePartsWidget->loadContact( contact );
  d->mNamePage.displayNameWidget->changeName( contact );
  blockSignals( false );
}

#include "moc_editormore.cpp"

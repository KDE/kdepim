/*
    This file is part of KitchenSync.

    Copyright (c) 2007 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2007 Anirudh Ramesh <abattoir@abattoir.in>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "configguisunbird.h"

#include <tqdom.h>
#include <tqtabwidget.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqbuttongroup.h>
#include <tqcheckbox.h>
#include <tqsizepolicy.h>
#include <tqptrlist.h>
#include <tqspinbox.h>
#include <tqwidget.h>

#include <kurlrequester.h>
#include <klineedit.h>
#include <kpushbutton.h>
#include <kdialog.h>
#include <klocale.h>
#include <kfile.h>

ConfigGuiSunbird::ConfigGuiSunbird( const QSync::Member &member, TQWidget *parent )
  : ConfigGui( member, parent )
{
  TQTabWidget *tabWidget = new TQTabWidget( this );
  topLayout()->addWidget( tabWidget );

  mLocalWidget = new TQWidget( tabWidget );
  mLocalLayout = new TQVBoxLayout( mLocalWidget, KDialog::spacingHint() );

  mWebdavWidget = new TQWidget( tabWidget );
  mWebdavLayout = new TQVBoxLayout( mWebdavWidget, KDialog::spacingHint() );

  tabWidget->addTab( mLocalWidget, i18n( "Local Calendars" ) );
  tabWidget->addTab( mWebdavWidget, i18n( "WebDAV Calendars" ) );

  KPushButton *mLocalAddButton = new KPushButton( mLocalWidget );
  mLocalAddButton->setText( i18n( "Add new calendar" ) );
  mLocalAddButton->setSizePolicy( TQSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed ) );
  mLocalLayout->addWidget( mLocalAddButton );
  connect( mLocalAddButton, TQT_SIGNAL( clicked() ),
           this, TQT_SLOT( addLocalCalendar() ) );

  KPushButton *mWebdavAddButton = new KPushButton( mWebdavWidget );
  mWebdavAddButton->setText( i18n( "Add new calendar" ) );
  mWebdavAddButton->setSizePolicy( TQSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed ) );
  mWebdavLayout->addWidget( mWebdavAddButton );
  connect( mWebdavAddButton, TQT_SIGNAL( clicked() ),
           this, TQT_SLOT( addWebdavCalendar() ) );

  mLocalSpacer = new TQSpacerItem( 20, 40, TQSizePolicy::Expanding );
  mLocalLayout->addItem( mLocalSpacer );
  mWebdavSpacer = new TQSpacerItem( 20, 40, TQSizePolicy::Expanding );
  mWebdavLayout->addItem( mWebdavSpacer );
}

void ConfigGuiSunbird::load( const TQString &xml )
{
  TQString path;
  TQString url;
  TQString username;
  TQString password;
  TQString defaultcal;
  TQString days;

  TQDomDocument doc;
  doc.setContent( xml );
  TQDomElement docElement = doc.documentElement();
  TQDomNode node;
  for( node = docElement.firstChild(); !node.isNull(); node = node.nextSibling() ) {
    TQDomElement element = node.toElement();
    if ( element.tagName() == "file" ) {
      TQDomAttr pathAttr = element.attributeNode( "path" );
      path = pathAttr.value();
      TQDomAttr defaultAttr = element.attributeNode( "default" );
      defaultcal = defaultAttr.value();
      TQDomAttr daysAttr = element.attributeNode( "deletedaysold" );
      days = daysAttr.value();

      LocalCalendar *cal = new LocalCalendar( path, defaultcal, days, mLocalWidget );
      mLocalLayout->removeItem( mLocalSpacer );
      cal->setSizePolicy( TQSizePolicy( TQSizePolicy::Expanding, TQSizePolicy::Fixed ) );
      mLocalLayout->addWidget( cal );
      mLocalLayout->addItem( mLocalSpacer );
      mLocalList.append( cal );

      connect( cal, TQT_SIGNAL( deleteRequest( LocalCalendar* ) ), TQT_SLOT( delLocalCalendar( LocalCalendar* ) ) );
      cal->show();
    } else if ( element.tagName() == "webdav" ) {
      TQDomAttr urlAttr = element.attributeNode( "url" );
      url = urlAttr.value();
      TQDomAttr unameAttr = element.attributeNode( "username" );
      username = unameAttr.value();
      TQDomAttr pwordAttr = element.attributeNode( "password" );
      password = pwordAttr.value();
      TQDomAttr defaultAttr = element.attributeNode( "default" );
      defaultcal = defaultAttr.value();
      TQDomAttr daysAttr = element.attributeNode( "deletedaysold" );
      days = daysAttr.value();

      WebdavCalendar *cal = new WebdavCalendar( username, password,
                                                url, defaultcal, days, mWebdavWidget );
      mWebdavLayout->removeItem( mWebdavSpacer );
      cal->setSizePolicy( TQSizePolicy( TQSizePolicy::Expanding, TQSizePolicy::Fixed ) );
      mWebdavLayout->addWidget( cal );
      mWebdavLayout->addItem( mWebdavSpacer );
      mWebdavList.append( cal );

      connect( cal, TQT_SIGNAL( deleteRequest( WebdavCalendar* ) ), TQT_SLOT( delWebdavCalendar( WebdavCalendar* ) ) );
      cal->show();
    }
  }
}

TQString ConfigGuiSunbird::save() const
{
  TQString config = "<config>\n";

  for ( uint i = 0; i < mLocalList.count(); ++i ) {
    LocalCalendar *lcal = mLocalList[ i ];
    config += TQString( "<file " );
    config += TQString( "path=\"%1\" " ).arg( lcal->mPathRequester->url() );

    if ( lcal->mDaysCheckBox->isChecked() ) {
      config += TQString( "deletedaysold=\"%1\" " ).arg( lcal->mDaysSpinBox->value() );
    }
    if ( lcal->mDefaultCheckBox->isChecked() ) {
      config += TQString( "default=\"1\" " );
    }
    config += TQString( "/>\n" );
  }

  for ( uint i = 0; i < mWebdavList.count(); ++i ) {
    WebdavCalendar *wcal = mWebdavList[ i ];
    config += TQString( "<webdav " );
    config += TQString( "username=\"%1\" " ).arg( wcal->mUsername->text() );
    config += TQString( "password=\"%1\" " ).arg( wcal->mPassword->text() );
    config += TQString( "url=\"%1\" " ).arg( wcal->mUrl->text() );

    if ( wcal->mDaysCheckBox->isChecked() ) {
      config += TQString( "deletedaysold=\"%1\" " ).arg( wcal->mDaysSpinBox->value() );
    }
    if ( wcal->mDefaultCheckBox->isChecked() ) {
      config += TQString( "default=\"1\" " );
    }
    config += TQString( "/>\n" );
  }
  config += "</config>";

  return config;
}

void ConfigGuiSunbird::addLocalCalendar()
{
  LocalCalendar *cal = new LocalCalendar( mLocalWidget );
  mLocalLayout->removeItem( mLocalSpacer );
  cal->setSizePolicy( TQSizePolicy( TQSizePolicy::Expanding, TQSizePolicy::Fixed ) );
  mLocalLayout->addWidget( cal );
  mLocalLayout->addItem( mLocalSpacer );
  mLocalList.append( cal );

  connect( cal, TQT_SIGNAL( deleteRequest( LocalCalendar* ) ), TQT_SLOT( delLocalCalendar( LocalCalendar* ) ) );
  cal->show();
}

void ConfigGuiSunbird::delLocalCalendar( LocalCalendar *calendar )
{
  mLocalList.remove( calendar );
  calendar->deleteLater();
}

void ConfigGuiSunbird::addWebdavCalendar()
{
  WebdavCalendar *cal = new WebdavCalendar( mWebdavWidget );
  mWebdavLayout->removeItem( mWebdavSpacer );
  cal->setSizePolicy( TQSizePolicy( TQSizePolicy::Expanding, TQSizePolicy::Fixed ) );
  mWebdavLayout->addWidget( cal );
  mWebdavLayout->addItem( mWebdavSpacer );
  mWebdavList.append( cal );

  connect( cal, TQT_SIGNAL( deleteRequest( WebdavCalendar* ) ), TQT_SLOT( delWebdavCalendar( WebdavCalendar* ) ) );
  cal->show();
}

void ConfigGuiSunbird::delWebdavCalendar( WebdavCalendar *calendar )
{
  mWebdavList.remove( calendar );
  calendar->deleteLater();
}

LocalCalendar::LocalCalendar( TQWidget *parent )
  : TQWidget( parent )
{
  initGui();
}

LocalCalendar::LocalCalendar( const TQString &path, const TQString &defaultcal, const TQString &days, TQWidget *parent )
  : TQWidget( parent )
{
  initGui();

  mPathRequester->setURL( path );
  mDefaultCheckBox->setChecked( defaultcal.toInt() == 1 );

  if ( !days.isEmpty() ) {
    mDaysCheckBox->setChecked( true );
    mDaysSpinBox->setEnabled( true );
    mDaysSpinBox->setValue( days.toInt() );
  }
}

void LocalCalendar::initGui()
{
  TQBoxLayout *bottomLayout = new TQHBoxLayout();

  mDaysCheckBox = new TQCheckBox( this );
  mDaysCheckBox->setText( i18n( "Sync only events newer than" ) );

  mDaysSpinBox = new TQSpinBox( this );
  mDaysSpinBox->setDisabled( true );
  mDaysSpinBox->setSizePolicy( TQSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed ) );

  connect( mDaysCheckBox, TQT_SIGNAL( toggled( bool ) ),
           this, TQT_SLOT( toggleDays( bool ) ) );

  bottomLayout->addWidget( mDaysCheckBox );
  bottomLayout->addWidget( mDaysSpinBox );
  bottomLayout->addWidget( new TQLabel( i18n( "day(s)" ), this ) );

  TQGridLayout *localLayout = new TQGridLayout( this );

  mPathRequester = new KURLRequester( this );

  KPushButton *removeButton = new KPushButton( this );
  removeButton->setSizePolicy( TQSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed ) );
  removeButton->setText( i18n( "Remove" ) );
  connect( removeButton, TQT_SIGNAL( clicked() ),
           this, TQT_SLOT( deleteWidget() ) );

  mDefaultCheckBox = new TQCheckBox( this );
  mDefaultCheckBox->setText( i18n( "Set as Default" ) );

  localLayout->addItem( new TQSpacerItem( 40, 20, TQSizePolicy::Expanding ), 0, 0 );
  localLayout->addWidget( new TQLabel( i18n( "Location:" ), this ), 1, 0 );
  localLayout->addWidget( mPathRequester, 1, 1 );
  localLayout->addItem( new TQSpacerItem( 40, 20, TQSizePolicy::Fixed ), 1, 2 );
  localLayout->addWidget( removeButton, 1, 3 );
  localLayout->addMultiCellLayout( bottomLayout, 2, 2, 0, 2 );
  localLayout->addWidget( mDefaultCheckBox, 2, 3 ); 
}

void LocalCalendar::deleteWidget()
{
  emit deleteRequest( this );
}

WebdavCalendar::WebdavCalendar( TQWidget *parent )
  : TQWidget( parent )
{
  initGui();
};

WebdavCalendar::WebdavCalendar( const TQString &username, const TQString &password, const TQString &url,
                                const TQString &defaultcal, const TQString &days, TQWidget *parent )
  : TQWidget( parent )
{
  initGui();

  mUsername->setText( username );
  mPassword->setText( password );
  mUrl->setText( url );
  mDefaultCheckBox->setChecked( defaultcal.toInt() == 1 );

  if ( !days.isEmpty() ) {
    mDaysCheckBox->setChecked( true );
    mDaysSpinBox->setEnabled( true );
    mDaysSpinBox->setValue( days.toInt() );
  }
}

void WebdavCalendar::initGui()
{
  TQBoxLayout *bottomLayout = new TQHBoxLayout();

  mDaysCheckBox = new TQCheckBox( this );
  mDaysCheckBox->setText( i18n( "Sync only events newer than" ) );

  mDaysSpinBox = new TQSpinBox( this );
  mDaysSpinBox->setDisabled( true );
  mDaysSpinBox->setSizePolicy( TQSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed ) );

  connect( mDaysCheckBox, TQT_SIGNAL( toggled( bool ) ),
           this, TQT_SLOT( toggleDays( bool ) ) );

  bottomLayout->addWidget( mDaysCheckBox );
  bottomLayout->addWidget( mDaysSpinBox );
  bottomLayout->addWidget( new TQLabel( i18n( "day(s)" ), this ) );

  TQGridLayout *webdavLayout = new TQGridLayout();

  mUrl = new KLineEdit( this );
  mUsername = new KLineEdit( this );
  mPassword = new KLineEdit( this );
  mPassword->setEchoMode( KLineEdit::Password );

  KPushButton *removeButton = new KPushButton( this );
  removeButton->setText( i18n( "Remove" ) );
  connect( removeButton, TQT_SIGNAL( clicked() ),
           this, TQT_SLOT( deleteWidget() ) );

  mDefaultCheckBox = new TQCheckBox( this );
  mDefaultCheckBox->setText( i18n( "Set as Default" ) );

  webdavLayout->addWidget( new TQLabel( i18n( "Location:" ), this ), 0, 0 );
  webdavLayout->addWidget( mUrl, 0, 1 );
  webdavLayout->addItem( new TQSpacerItem( 40, 20, TQSizePolicy::Fixed ), 0, 2 );
  webdavLayout->addWidget( removeButton, 0, 3 );
  webdavLayout->addMultiCellLayout( bottomLayout, 1, 1, 0, 1 );
  webdavLayout->addWidget( mDefaultCheckBox, 1, 3 );

  TQGridLayout *mainLayout = new TQGridLayout( this );
  mainLayout->addItem( new TQSpacerItem( 40, 20, TQSizePolicy::Fixed ), 0, 0 );
  mainLayout->addMultiCellLayout( webdavLayout, 1, 1, 0, 4 );
  mainLayout->addWidget( new TQLabel( i18n( "Username:" ), this ), 2, 0 );
  mainLayout->addWidget( mUsername, 2, 1 );
  mainLayout->addItem( new TQSpacerItem( 40, 20, TQSizePolicy::Fixed ), 2, 2 );
  mainLayout->addWidget( new TQLabel( i18n( "Password:" ), this ), 2, 3 );
  mainLayout->addWidget( mPassword, 2, 4 );
}

void WebdavCalendar::deleteWidget()
{
  emit deleteRequest( this );
}

void LocalCalendar::toggleDays( bool state )
{
  mDaysSpinBox->setEnabled( state );
}

void WebdavCalendar::toggleDays( bool state )
{
  mDaysSpinBox->setEnabled( state );
}

#include "configguisunbird.moc"

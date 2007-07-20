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

#include <qdom.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qsizepolicy.h>
#include <qptrlist.h>
#include <qspinbox.h>
#include <qwidget.h>

#include <kurlrequester.h>
#include <klineedit.h>
#include <kpushbutton.h>
#include <kdialog.h>
#include <klocale.h>
#include <kfile.h>

ConfigGuiSunbird::ConfigGuiSunbird( const QSync::Member &member, QWidget *parent )
  : ConfigGui( member, parent )
{
  QTabWidget *tabWidget = new QTabWidget( this );
  topLayout()->addWidget( tabWidget );

  mLocalWidget = new QWidget( tabWidget );
  mLocalLayout = new QVBoxLayout( mLocalWidget, KDialog::spacingHint() );

  mWebdavWidget = new QWidget( tabWidget );
  mWebdavLayout = new QVBoxLayout( mWebdavWidget, KDialog::spacingHint() );

  tabWidget->addTab( mLocalWidget, i18n( "Local Calendars" ) );
  tabWidget->addTab( mWebdavWidget, i18n( "WebDAV Calendars" ) );

  KPushButton *mLocalAddButton = new KPushButton( mLocalWidget );
  mLocalAddButton->setText( i18n( "Add new calendar" ) );
  mLocalAddButton->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
  mLocalLayout->addWidget( mLocalAddButton );
  connect( mLocalAddButton, SIGNAL( clicked() ),
           this, SLOT( addLocalCalendar() ) );

  KPushButton *mWebdavAddButton = new KPushButton( mWebdavWidget );
  mWebdavAddButton->setText( i18n( "Add new calendar" ) );
  mWebdavAddButton->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
  mWebdavLayout->addWidget( mWebdavAddButton );
  connect( mWebdavAddButton, SIGNAL( clicked() ),
           this, SLOT( addWebdavCalendar() ) );

  mLocalSpacer = new QSpacerItem( 20, 40, QSizePolicy::Expanding );
  mLocalLayout->addItem( mLocalSpacer );
  mWebdavSpacer = new QSpacerItem( 20, 40, QSizePolicy::Expanding );
  mWebdavLayout->addItem( mWebdavSpacer );
}

void ConfigGuiSunbird::load( const QString &xml )
{
  QString path;
  QString url;
  QString username;
  QString password;
  QString defaultcal;
  QString days;

  QDomDocument doc;
  doc.setContent( xml );
  QDomElement docElement = doc.documentElement();
  QDomNode node;
  for( node = docElement.firstChild(); !node.isNull(); node = node.nextSibling() ) {
    QDomElement element = node.toElement();
    if ( element.tagName() == "file" ) {
      QDomAttr pathAttr = element.attributeNode( "path" );
      path = pathAttr.value();
      QDomAttr defaultAttr = element.attributeNode( "default" );
      defaultcal = defaultAttr.value();
      QDomAttr daysAttr = element.attributeNode( "deletedaysold" );
      days = daysAttr.value();

      LocalCalendar *cal = new LocalCalendar( path, defaultcal, days, mLocalWidget );
      mLocalLayout->removeItem( mLocalSpacer );
      cal->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
      mLocalLayout->addWidget( cal );
      mLocalLayout->addItem( mLocalSpacer );
      mLocalList.append( cal );

      connect( cal, SIGNAL( deleteRequest( LocalCalendar* ) ), SLOT( delLocalCalendar( LocalCalendar* ) ) );
      cal->show();
    } else if ( element.tagName() == "webdav" ) {
      QDomAttr urlAttr = element.attributeNode( "url" );
      url = urlAttr.value();
      QDomAttr unameAttr = element.attributeNode( "username" );
      username = unameAttr.value();
      QDomAttr pwordAttr = element.attributeNode( "password" );
      password = pwordAttr.value();
      QDomAttr defaultAttr = element.attributeNode( "default" );
      defaultcal = defaultAttr.value();
      QDomAttr daysAttr = element.attributeNode( "deletedaysold" );
      days = daysAttr.value();

      WebdavCalendar *cal = new WebdavCalendar( username, password,
                                                url, defaultcal, days, mWebdavWidget );
      mWebdavLayout->removeItem( mWebdavSpacer );
      cal->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
      mWebdavLayout->addWidget( cal );
      mWebdavLayout->addItem( mWebdavSpacer );
      mWebdavList.append( cal );

      connect( cal, SIGNAL( deleteRequest( WebdavCalendar* ) ), SLOT( delWebdavCalendar( WebdavCalendar* ) ) );
      cal->show();
    }
  }
}

QString ConfigGuiSunbird::save() const
{
  QString config = "<config>\n";

  for ( uint i = 0; i < mLocalList.count(); ++i ) {
    LocalCalendar *lcal = mLocalList[ i ];
    config += QString( "<file " );
    config += QString( "path=\"%1\" " ).arg( lcal->mPathRequester->url() );

    if ( lcal->mDaysCheckBox->isChecked() ) {
      config += QString( "deletedaysold=\"%1\" " ).arg( lcal->mDaysSpinBox->value() );
    }
    if ( lcal->mDefaultCheckBox->isChecked() ) {
      config += QString( "default=\"1\" " );
    }
    config += QString( "/>\n" );
  }

  for ( uint i = 0; i < mWebdavList.count(); ++i ) {
    WebdavCalendar *wcal = mWebdavList[ i ];
    config += QString( "<webdav " );
    config += QString( "username=\"%1\" " ).arg( wcal->mUsername->text() );
    config += QString( "password=\"%1\" " ).arg( wcal->mPassword->text() );
    config += QString( "url=\"%1\" " ).arg( wcal->mUrl->text() );

    if ( wcal->mDaysCheckBox->isChecked() ) {
      config += QString( "deletedaysold=\"%1\" " ).arg( wcal->mDaysSpinBox->value() );
    }
    if ( wcal->mDefaultCheckBox->isChecked() ) {
      config += QString( "default=\"1\" " );
    }
    config += QString( "/>\n" );
  }
  config += "</config>";

  return config;
}

void ConfigGuiSunbird::addLocalCalendar()
{
  LocalCalendar *cal = new LocalCalendar( mLocalWidget );
  mLocalLayout->removeItem( mLocalSpacer );
  cal->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
  mLocalLayout->addWidget( cal );
  mLocalLayout->addItem( mLocalSpacer );
  mLocalList.append( cal );

  connect( cal, SIGNAL( deleteRequest( LocalCalendar* ) ), SLOT( delLocalCalendar( LocalCalendar* ) ) );
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
  cal->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
  mWebdavLayout->addWidget( cal );
  mWebdavLayout->addItem( mWebdavSpacer );
  mWebdavList.append( cal );

  connect( cal, SIGNAL( deleteRequest( WebdavCalendar* ) ), SLOT( delWebdavCalendar( WebdavCalendar* ) ) );
  cal->show();
}

void ConfigGuiSunbird::delWebdavCalendar( WebdavCalendar *calendar )
{
  mWebdavList.remove( calendar );
  calendar->deleteLater();
}

LocalCalendar::LocalCalendar( QWidget *parent )
  : QWidget( parent )
{
  initGui();
}

LocalCalendar::LocalCalendar( const QString &path, const QString &defaultcal, const QString &days, QWidget *parent )
  : QWidget( parent )
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
  QBoxLayout *bottomLayout = new QHBoxLayout();

  mDaysCheckBox = new QCheckBox( this );
  mDaysCheckBox->setText( i18n( "Sync only events newer than" ) );

  mDaysSpinBox = new QSpinBox( this );
  mDaysSpinBox->setDisabled( true );
  mDaysSpinBox->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );

  connect( mDaysCheckBox, SIGNAL( toggled( bool ) ),
           this, SLOT( toggleDays( bool ) ) );

  bottomLayout->addWidget( mDaysCheckBox );
  bottomLayout->addWidget( mDaysSpinBox );
  bottomLayout->addWidget( new QLabel( i18n( "day(s)" ), this ) );

  QGridLayout *localLayout = new QGridLayout( this );

  mPathRequester = new KURLRequester( this );

  KPushButton *removeButton = new KPushButton( this );
  removeButton->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
  removeButton->setText( i18n( "Remove" ) );
  connect( removeButton, SIGNAL( clicked() ),
           this, SLOT( deleteWidget() ) );

  mDefaultCheckBox = new QCheckBox( this );
  mDefaultCheckBox->setText( i18n( "Set as Default" ) );

  localLayout->addItem( new QSpacerItem( 40, 20, QSizePolicy::Expanding ), 0, 0 );
  localLayout->addWidget( new QLabel( i18n( "Location:" ), this ), 1, 0 );
  localLayout->addWidget( mPathRequester, 1, 1 );
  localLayout->addItem( new QSpacerItem( 40, 20, QSizePolicy::Fixed ), 1, 2 );
  localLayout->addWidget( removeButton, 1, 3 );
  localLayout->addMultiCellLayout( bottomLayout, 2, 2, 0, 2 );
  localLayout->addWidget( mDefaultCheckBox, 2, 3 ); 
}

void LocalCalendar::deleteWidget()
{
  emit deleteRequest( this );
}

WebdavCalendar::WebdavCalendar( QWidget *parent )
  : QWidget( parent )
{
  initGui();
};

WebdavCalendar::WebdavCalendar( const QString &username, const QString &password, const QString &url,
                                const QString &defaultcal, const QString &days, QWidget *parent )
  : QWidget( parent )
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
  QBoxLayout *bottomLayout = new QHBoxLayout();

  mDaysCheckBox = new QCheckBox( this );
  mDaysCheckBox->setText( i18n( "Sync only events newer than" ) );

  mDaysSpinBox = new QSpinBox( this );
  mDaysSpinBox->setDisabled( true );
  mDaysSpinBox->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );

  connect( mDaysCheckBox, SIGNAL( toggled( bool ) ),
           this, SLOT( toggleDays( bool ) ) );

  bottomLayout->addWidget( mDaysCheckBox );
  bottomLayout->addWidget( mDaysSpinBox );
  bottomLayout->addWidget( new QLabel( i18n( "day(s)" ), this ) );

  QGridLayout *webdavLayout = new QGridLayout();

  mUrl = new KLineEdit( this );
  mUsername = new KLineEdit( this );
  mPassword = new KLineEdit( this );
  mPassword->setEchoMode( KLineEdit::Password );

  KPushButton *removeButton = new KPushButton( this );
  removeButton->setText( i18n( "Remove" ) );
  connect( removeButton, SIGNAL( clicked() ),
           this, SLOT( deleteWidget() ) );

  mDefaultCheckBox = new QCheckBox( this );
  mDefaultCheckBox->setText( i18n( "Set as Default" ) );

  webdavLayout->addWidget( new QLabel( i18n( "Location:" ), this ), 0, 0 );
  webdavLayout->addWidget( mUrl, 0, 1 );
  webdavLayout->addItem( new QSpacerItem( 40, 20, QSizePolicy::Fixed ), 0, 2 );
  webdavLayout->addWidget( removeButton, 0, 3 );
  webdavLayout->addMultiCellLayout( bottomLayout, 1, 1, 0, 1 );
  webdavLayout->addWidget( mDefaultCheckBox, 1, 3 );

  QGridLayout *mainLayout = new QGridLayout( this );
  mainLayout->addItem( new QSpacerItem( 40, 20, QSizePolicy::Fixed ), 0, 0 );
  mainLayout->addMultiCellLayout( webdavLayout, 1, 1, 0, 4 );
  mainLayout->addWidget( new QLabel( i18n( "Username:" ), this ), 2, 0 );
  mainLayout->addWidget( mUsername, 2, 1 );
  mainLayout->addItem( new QSpacerItem( 40, 20, QSizePolicy::Fixed ), 2, 2 );
  mainLayout->addWidget( new QLabel( i18n( "Password:" ), this ), 2, 3 );
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

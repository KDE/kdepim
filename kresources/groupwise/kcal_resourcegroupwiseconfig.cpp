/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <typeinfo>

#include <QLabel>
#include <QLayout>
#include <QCheckBox>
//Added by qt3to4:
#include <QGridLayout>

#include <klocale.h>
#include <k3listview.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kstandarddirs.h>
#include <klineedit.h>

#include <kcal/resourcecachedconfig.h>

#include "kcal_resourcegroupwise.h"
#include "kcal_groupwiseprefs.h"
#include "kcal_resourcegroupwiseconfig.h"
#include "groupwisesettingswidget.h"
#include "soap/soapH.h"

using namespace KCal;

ResourceGroupwiseConfig::ResourceGroupwiseConfig( QWidget* parent )
    : KRES::ConfigWidget( parent )
{
  resize( 245, 115 );
  QGridLayout *mainLayout = new QGridLayout( this );

  QLabel *label = new QLabel( i18n("URL:"), this );
  mainLayout->addWidget( label, 1, 0 );
  mUrl = new KLineEdit( this );
  mainLayout->addWidget( mUrl, 1, 1, 1, 3 );

  label = new QLabel( i18n("User:"), this );
  mainLayout->addWidget( label, 2, 0 );
  mUserEdit = new KLineEdit( this );
  mainLayout->addWidget( mUserEdit, 2, 1, 1, 3 );

  label = new QLabel( i18n("Password:"), this );
  mainLayout->addWidget( label, 3, 0 );
  mPasswordEdit = new KLineEdit( this );
  mainLayout->addWidget( mPasswordEdit, 3, 1, 1, 3 );
  mPasswordEdit->setEchoMode( KLineEdit::Password );

  QPushButton *settingsButton = new QPushButton( i18n( "View User Settings" ), this );
  mainLayout->addWidget( settingsButton, 4, 0, 1, 4 );

  mReloadConfig = new KCal::ResourceCachedReloadConfig( this );
  mainLayout->addWidget( mReloadConfig, 5, 0, 1, 2 );

  mSaveConfig = new KCal::ResourceCachedSaveConfig( this );
  mainLayout->addWidget( mSaveConfig, 5, 2, 1, 2 );

  settingsButton->hide();
  // connect( settingsButton, SIGNAL( clicked() ), SLOT( slotViewUserSettings() ) );

}

void ResourceGroupwiseConfig::loadSettings( KRES::Resource *resource )
{
  kDebug() <<"KCal::ResourceGroupwiseConfig::loadSettings()";
  ResourceGroupwise *res = static_cast<ResourceGroupwise *>( resource );
  mResource = res;

  if ( res ) {
    if ( !res->prefs() ) {
      kError() <<"No PREF";
      return;
    }

    mUrl->setText( res->prefs()->url() );
    mUserEdit->setText( res->prefs()->user() );
    mPasswordEdit->setText( res->prefs()->password() );
    mReloadConfig->loadSettings( res );
    mSaveConfig->loadSettings( res );
  } else {
    kError(5700) <<"KCalResourceGroupwiseConfig::loadSettings(): no KCalResourceGroupwise, cast failed";
  }
}

void ResourceGroupwiseConfig::saveSettings( KRES::Resource *resource )
{
  ResourceGroupwise *res = static_cast<ResourceGroupwise*>( resource );
  if ( res ) {
    res->prefs()->setUrl( mUrl->text() );
    res->prefs()->setUser( mUserEdit->text() );
    res->prefs()->setPassword( mPasswordEdit->text() );
    mReloadConfig->saveSettings( res );
    mSaveConfig->saveSettings( res );
  } else {
    kError(5700) <<"KCalResourceGroupwiseConfig::saveSettings(): no KCalResourceGroupwise, cast failed";
  }
}

void ResourceGroupwiseConfig::slotViewUserSettings()
{
  kDebug(5700) <<"KCal::ResourceGroupwiseConfig::slotViewUserSettings()";
  if ( mResource )
  {
    ngwt__Settings * s;
    mResource->userSettings( s );

    if ( s )
    {
      KDialog * dialog = new KDialog( qobject_cast<QWidget*>(parent() ) );
	  dialog->setCaption(i18n( "GroupWise Settings" ) );
	  dialog->setModal(true);
//       QVBoxLayout * layout = new QVBoxLayout( dialog );
      QWidget * wid = new QWidget( dialog );
      GroupWiseSettingsWidget settingsWidget;
      settingsWidget.setupUi( wid );
      dialog->setMainWidget( wid );
      // populate dialog
      kDebug() <<"slotViewUserSettings() - settings are:";
      std::vector<class ngwt__SettingsGroup *>::const_iterator it;
      for( it = s->group.begin(); it != s->group.end(); ++it )
      {
        ngwt__SettingsGroup * group = *it;
        QString groupName;
        if ( group->type )
        {
          groupName = QString::fromUtf8( group->type->c_str() );
          kDebug() <<"GROUP:" << groupName;;
        }
        K3ListViewItem * groupLVI = new K3ListViewItem( settingsWidget.m_settingsList, groupName );
        std::vector<ngwt__Custom * > setting = group->setting;
        std::vector<class ngwt__Custom *>::const_iterator it2;
        for( it2 = setting.begin(); it2 != setting.end(); ++it2 )
        {
          QString setting, value;
          bool locked = false;
          setting = QString::fromUtf8( (*it2)->field.c_str() );
          if ( (*it2)->value )
          {
            value = QString::fromUtf8( (*it2)->value->c_str() );
          }
          if ( (*it2)->locked )
            locked = *((*it2)->locked);

          kDebug() <<"  SETTING:" << setting  <<"   value :" << value <<  (locked ?"locked" :" not locked" );
          K3ListViewItem * settingLVI = new K3ListViewItem( groupLVI, QString(), setting, value, (locked ? "locked" : " not locked " ) );
          if ( !locked )
            settingLVI->setRenameEnabled( 2, true );
        }
      }

      dialog->show();
      if ( dialog->exec() == QDialog::Accepted )
      {
        QMap<QString, QString> settings = settingsWidget.dirtySettings();
        mResource->modifyUserSettings( settings );
      }
    }
      else
        kDebug() <<"KCal::ResourceGroupwiseConfig::slotViewUserSettings() - NO SETTINGS";
  }
}

#include "kcal_resourcegroupwiseconfig.moc"

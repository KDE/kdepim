/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Cornelius Schumacher <schumacher@kde.org>

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

#include "configgui.h"


#include "configguiblank.h"
#include "configguifile.h"
#include "configguignokii.h"
#include "configguigpe.h"
#include "configguiirmc.h"
#include "configguildap.h"
#include "configguiopie.h"
#include "configguipalm.h"
#include "configguisyncmlhttp.h"
#include "configguisyncmlobex.h"
#include "configguigcalendar.h"
#include "configguijescs.h"
#include "configguievo2.h"
#include "configguimoto.h"
#include "configguisynce.h"
#include "configguisunbird.h"

#include "memberinfo.h"

#include <kdialog.h>
#include <klocale.h>
#include <klineedit.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qtextedit.h>

ConfigGui::ConfigGui( const QSync::Member &member, QWidget *parent )
  : QWidget( parent ), mMember( member )
{
  mTopLayout = new QVBoxLayout( this );
  mTopLayout->setSpacing( KDialog::spacingHint() );
  mTopLayout->setMargin( KDialog::marginHint() );

  QBoxLayout *nameLayout = new QHBoxLayout( mTopLayout );

  QLabel *label = new QLabel( i18n("Name:"), this );
  nameLayout->addWidget( label );

  mNameEdit = new KLineEdit( this );
  nameLayout->addWidget( mNameEdit );
}

void ConfigGui::setInstanceName( const QString &t )
{
  mNameEdit->setText( t );
}

QString ConfigGui::instanceName() const
{
  return mNameEdit->text();
}

ConfigGui *ConfigGui::Factory::create( const QSync::Member &member,
  QWidget *parent )
{
  QString name = member.pluginName();
  if ( name == "file-sync" ) {
    return new ConfigGuiFile( member, parent );
  } else if ( name == "palm-sync" ) {
    return new ConfigGuiPalm( member, parent );
  } else if ( name == "irmc-sync" ) {
    return new ConfigGuiIRMC( member, parent );
  } else if ( name == "syncml-obex-client" ) {
    return new ConfigGuiSyncmlObex( member, parent );
  } else if ( name == "syncml-http-server" ) {
    return new ConfigGuiSyncmlHttp( member, parent );
  } else if ( name == "opie-sync" ) {
    return new ConfigGuiOpie( member, parent );
  } else if ( name == "gnokii-sync" ) {
    return new ConfigGuiGnokii( member, parent );
  } else if ( name == "gpe-sync" ) {
    return new ConfigGuiGpe( member, parent );
  } else if ( name == "google-calendar" ) {
    return new ConfigGuiGoogleCalendar( member, parent );
  } else if ( name == "ldap-sync" ) {
    return new ConfigGuiLdap( member, parent );
  } else if ( name == "kdepim-sync" ) {
    return new ConfigGuiBlank( member, parent ); 
  } else if ( name == "jescs-sync" ) {
    return new ConfigGuiJescs( member, parent );
  } else if ( name == "evo2-sync" ) {
    return new ConfigGuiEvo2( member, parent );
  } else if ( name == "moto-sync" ) {
    return new ConfigGuiMoto( member, parent );
  } else if ( name == "synce-plugin" ) {
    return new ConfigGuiSynce( member, parent );
  } else if ( name == "sunbird-sync" ) {
    return new ConfigGuiSunbird( member, parent );
  } else {
    return new ConfigGuiXml( member, parent );
  }
}


ConfigGuiXml::ConfigGuiXml( const QSync::Member &member, QWidget *parent )
  : ConfigGui( member, parent )
{
  mTextEdit = new QTextEdit( this );
  topLayout()->addWidget( mTextEdit );  
}

void ConfigGuiXml::load( const QString &xml )
{
  mTextEdit->setText( xml );
}

QString ConfigGuiXml::save() const
{
  return mTextEdit->text();
}

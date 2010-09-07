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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "memberconfig.h"

#include "configgui.h"
#include "memberinfo.h"

#include <libqopensync/result.h>

#include <klocale.h>
#include <kmessagebox.h>

#include <tqlabel.h>
#include <tqlayout.h>
#include <tqtabwidget.h>

MemberConfig::MemberConfig( TQWidget *parent, const QSync::Member &member )
  : TQWidget( parent ), mMember( member )
{
  TQBoxLayout *topLayout = new TQVBoxLayout( this );

  mGui = ConfigGui::Factory::create( member, this );
  topLayout->addWidget( mGui );
}

MemberConfig::~MemberConfig()
{
}

void MemberConfig::loadData()
{
  TQByteArray cfg;
  QSync::Result error = mMember.configuration( cfg );

  if ( error ) {
    KMessageBox::error( this,
      i18n("Unable to read config from plugin '%1':\n%2")
      .arg( mMember.pluginName() ).arg( error.message() ) );
  } else {
    TQString txt = TQString::fromUtf8( cfg.data(), cfg.size() );
    mGui->load( txt );
    MemberInfo mi( mMember );
    mGui->setInstanceName( mi.name() );
  }
}

void MemberConfig::saveData()
{
  TQString txt = mGui->save();

  if ( txt.isEmpty() ) {
    KMessageBox::sorry( this, i18n("Configuration of %1 is empty.").arg( mMember.pluginName() ) );
  } else {
    mMember.setConfiguration( txt.utf8() );
    mMember.setName( mGui->instanceName() );
    // TODO: Check for save() error.
    mMember.save();
  }
}

QSync::Member MemberConfig::member() const
{
  return mMember;
}

#include "memberconfig.moc"

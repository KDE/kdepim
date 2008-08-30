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

#include <QtGui/QLayout>

#include "configwidget.h"
#include "memberinfo.h"

MemberConfig::MemberConfig( QWidget *parent, const QSync::Member &member )
  : QWidget( parent ), mMember( member )
{
  QBoxLayout *layout = new QVBoxLayout( this );

  mConfig = new ConfigWidget( member.configurationOrDefault(), this );
  layout->addWidget( mConfig );
}

MemberConfig::~MemberConfig()
{
}

void MemberConfig::loadData()
{
  mConfig->load();
  MemberInfo mi( mMember );
  mConfig->setInstanceName( mi.name() );
}

void MemberConfig::saveData()
{
  mConfig->save();
  mMember.setName( mConfig->instanceName() );
  mMember.save();
}

QSync::Member MemberConfig::member() const
{
  return mMember;
}

#include "memberconfig.moc"

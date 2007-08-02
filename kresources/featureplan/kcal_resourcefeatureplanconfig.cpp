/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QLabel>
#include <QLayout>
#include <QCheckBox>
//Added by qt3to4:
#include <QGridLayout>

#include <kdebug.h>
#include <kdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <knuminput.h>
#include <kurlrequester.h>

#include "kcal_resourcefeatureplan.h"
#include "kcal_resourcefeatureplanconfig.h"

using namespace KCal;

ResourceFeaturePlanConfig::ResourceFeaturePlanConfig( QWidget *parent )
  : KRES::ConfigWidget( parent )
{
  QGridLayout *topLayout = new QGridLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );
  topLayout->setMargin( 0 );

  QLabel *label = new QLabel( i18n( "Filename:" ), this );
  mFilename = new KUrlRequester( this );

  topLayout->addWidget( label, 0, 0 );
  topLayout->addWidget( mFilename, 0, 1 );

  label = new QLabel( i18n( "Filter email:" ), this );
  mFilterEmail = new KLineEdit( this );

  topLayout->addWidget( label, 1, 0 );
  topLayout->addWidget( mFilterEmail, 1, 1 );

  mCvsCheck = new QCheckBox( i18n("Use CVS"), this );

  topLayout->addWidget( mCvsCheck, 2, 0, 1, 2 );
}

void ResourceFeaturePlanConfig::loadSettings( KRES::Resource *res )
{
  ResourceFeaturePlan *resource = dynamic_cast<ResourceFeaturePlan *>( res );

  if ( !resource ) {
    kDebug(5700) <<"ResourceFeaturePlanConfig::loadSettings(): cast failed";
    return;
  }

  Prefs *p = resource->prefs();
  mFilename->setUrl( p->filename() );
  mFilterEmail->setText( p->filterEmail() );
  mCvsCheck->setChecked( p->useCvs() );
}

void ResourceFeaturePlanConfig::saveSettings( KRES::Resource *res )
{
  ResourceFeaturePlan *resource = dynamic_cast<ResourceFeaturePlan *>( res );

  if ( !resource ) {
    kDebug(5700) <<"ResourceFeaturePlanConfig::saveSettings(): cast failed";
    return;
  }

  Prefs *p = resource->prefs();
  p->setFilename( mFilename->url().url() );
  p->setFilterEmail( mFilterEmail->text() );
  p->setUseCvs( mCvsCheck->isChecked() );
}

#include "kcal_resourcefeatureplanconfig.moc"

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>

#include <kdebug.h>
#include <kdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <knuminput.h>
#include <kurlrequester.h>

#include "kcal_resourcefeatureplan.h"
#include "kcal_resourcefeatureplanconfig.h"

using namespace KCal;

ResourceFeaturePlanConfig::ResourceFeaturePlanConfig( QWidget *parent,
                                                      const char *name )
  : KRES::ConfigWidget( parent, name )
{
  QGridLayout *topLayout = new QGridLayout( this, 3, 2, 0,
                                            KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n( "Filename:" ), this );
  mFilename = new KURLRequester( this );

  topLayout->addWidget( label, 0, 0 );
  topLayout->addWidget( mFilename, 0, 1 );

  label = new QLabel( i18n( "Filter email:" ), this );
  mFilterEmail = new KLineEdit( this );

  topLayout->addWidget( label, 1, 0 );
  topLayout->addWidget( mFilterEmail, 1, 1 );

  mCvsCheck = new QCheckBox( i18n("Use CVS"), this );

  topLayout->addMultiCellWidget( mCvsCheck, 2, 2, 0, 1 );
}

void ResourceFeaturePlanConfig::loadSettings( KRES::Resource *res )
{
  ResourceFeaturePlan *resource = dynamic_cast<ResourceFeaturePlan *>( res );
  
  if ( !resource ) {
    kdDebug(5700) << "ResourceFeaturePlanConfig::loadSettings(): cast failed" << endl;
    return;
  }

  Prefs *p = resource->prefs();
  mFilename->setURL( p->filename() );
  mFilterEmail->setText( p->filterEmail() );
  mCvsCheck->setChecked( p->useCvs() );
}

void ResourceFeaturePlanConfig::saveSettings( KRES::Resource *res )
{
  ResourceFeaturePlan *resource = dynamic_cast<ResourceFeaturePlan *>( res );
  
  if ( !resource ) {
    kdDebug(5700) << "ResourceFeaturePlanConfig::saveSettings(): cast failed" << endl;
    return;
  }

  Prefs *p = resource->prefs();
  p->setFilename( mFilename->url() );
  p->setFilterEmail( mFilterEmail->text() );
  p->setUseCvs( mCvsCheck->isChecked() );
}

#include "kcal_resourcefeatureplanconfig.moc"

/*                                                                      
    This file is part of KAddressBook.                                  
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
                                                                        
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#include <qcheckbox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlayout.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>

#include "kabprefs.h"

#include "kabconfigwidget.h"

KABConfigWidget::KABConfigWidget( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QVBoxLayout *topLayout = new QVBoxLayout( this, KDialog::marginHint(),
                                            KDialog::spacingHint() );

  QGroupBox *groupBox = new QGroupBox( 0, Qt::Vertical, i18n( "General" ), this );
  QVBoxLayout *boxLayout = new QVBoxLayout( groupBox->layout() );
  boxLayout->setAlignment( Qt::AlignTop );

  mViewsSingleClickBox = new QCheckBox( i18n( "Honor KDE single click" ), groupBox, "msingle" );
  boxLayout->addWidget( mViewsSingleClickBox );

  mNameParsing = new QCheckBox( i18n( "Automatic name parsing for new addressees" ), groupBox, "mparse" );
  boxLayout->addWidget( mNameParsing );

  topLayout->addWidget( groupBox );

  connect( mNameParsing, SIGNAL( toggled( bool ) ), this, SLOT( modified() ) );
  connect( mViewsSingleClickBox, SIGNAL( toggled( bool ) ), this, SLOT( modified() ) );
}

void KABConfigWidget::restoreSettings()
{
  bool blocked = signalsBlocked();
  blockSignals( true );

  mNameParsing->setChecked( KABPrefs::instance()->mAutomaticNameParsing );
  mViewsSingleClickBox->setChecked( KABPrefs::instance()->mHonorSingleClick );

  blockSignals( blocked );

  emit changed( false );
}

void KABConfigWidget::saveSettings()
{
  kdDebug() << "KABConfigWidget::save()" << endl;

  KABPrefs::instance()->mAutomaticNameParsing = mNameParsing->isChecked();
  KABPrefs::instance()->mHonorSingleClick = mViewsSingleClickBox->isChecked();

  KABPrefs::instance()->writeConfig();

  emit changed( false );
}

void KABConfigWidget::defaults()
{
  mNameParsing->setChecked( true );
  mViewsSingleClickBox->setChecked( false );

  emit changed( true );
}

void KABConfigWidget::modified()
{
  emit changed( true );
}

#include "kabconfigwidget.moc"

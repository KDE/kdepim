/*
    This file is part of KitchenSync.
    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

    Based on the code of KRES::ConfigDialog from kdelibs

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

#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtabwidget.h>

#include <kresources/factory.h>

#include "konnector.h"

#include "konnectorconfigdialog.h"

KonnectorConfigDialog::KonnectorConfigDialog( QWidget *parent, KSync::Konnector *konnector )
  : KDialogBase( Plain, i18n( "Konnector Configuration" ), Ok | Cancel, Ok,
                 parent, "KonnectorConfigDialog", true, true ),
    mKonnector( konnector )
{
  QFrame *page = plainPage();

  QVBoxLayout *layout = new QVBoxLayout( page, 0, spacingHint() );

  QTabWidget *tab = new QTabWidget( page );
  tab->addTab( createGeneralPage( tab ), i18n( "General" ) );
  tab->addTab( createFilterPage( tab ), i18n( "Filter" ) );

  layout->addWidget( tab );

  setMinimumSize( sizeHint() );
}

void KonnectorConfigDialog::setInEditMode( bool value )
{
  if ( mConfigWidget )
    mConfigWidget->setInEditMode( value );
}

void KonnectorConfigDialog::slotNameChanged( const QString &text)
{
  enableButtonOK( !text.isEmpty() );
}

void KonnectorConfigDialog::setReadOnly( bool value )
{
  mReadOnly->setChecked( value );
}

void KonnectorConfigDialog::accept()
{
  if ( mName->text().isEmpty() ) {
    KMessageBox::sorry( this, i18n( "Please enter a resource name." ) );
    return;
  }

  mKonnector->setResourceName( mName->text() );
  mKonnector->setReadOnly( mReadOnly->isChecked() );

  if ( mConfigWidget ) {
    // First save generic information
    // Also save setting of specific resource type
    mConfigWidget->saveSettings( mKonnector );
  }

  KDialog::accept();
}

QWidget *KonnectorConfigDialog::createGeneralPage( QWidget *parent )
{
  QWidget *page = new QWidget( parent );
  QVBoxLayout *layout = new QVBoxLayout( page, marginHint(), spacingHint() );

  KRES::Factory *factory = KRES::Factory::self( "konnector" );

  QGroupBox *generalGroupBox = new QGroupBox( 2, Qt::Horizontal, page );
  generalGroupBox->layout()->setSpacing( spacingHint() );
  generalGroupBox->setTitle( i18n( "General Settings" ) );

  new QLabel( i18n( "Name:" ), generalGroupBox );

  mName = new KLineEdit( generalGroupBox );

  mReadOnly = new QCheckBox( i18n( "Read-only" ), generalGroupBox );

  mName->setText( mKonnector->resourceName() );
  mReadOnly->setChecked( mKonnector->readOnly() );

  layout->addWidget( generalGroupBox );

  QGroupBox *resourceGroupBox = new QGroupBox( 2, Qt::Horizontal,  page );
  resourceGroupBox->layout()->setSpacing( spacingHint() );
  resourceGroupBox->setTitle( i18n( "%1 Settings" )
                              .arg( factory->typeName( mKonnector->type() ) ) );
  layout->addWidget( resourceGroupBox );

  layout->addStretch();

  mConfigWidget = factory->configWidget( mKonnector->type(), resourceGroupBox );
  if ( mConfigWidget ) {
    mConfigWidget->setInEditMode( false );
    mConfigWidget->loadSettings( mKonnector );
    mConfigWidget->show();
    connect( mConfigWidget, SIGNAL( setReadOnly( bool ) ),
             this, SLOT( setReadOnly( bool ) ) );
  }

  connect( mName, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( slotNameChanged( const QString& ) ) );

  slotNameChanged( mName->text() );

  return page;
}

QWidget *KonnectorConfigDialog::createFilterPage( QWidget *parent )
{
  QWidget *page = new QWidget( parent );
  QVBoxLayout *layout = new QVBoxLayout( page, marginHint(), spacingHint() );

  const KSync::Filter::List filters = mKonnector->filters();
  KSync::Filter::List::ConstIterator it;
  for ( it = filters.begin(); it != filters.end(); ++it ) {
    QWidget *wdg = (*it)->configWidget( page );
    layout->addWidget( wdg );
  }

  return page;
}

#include "konnectorconfigdialog.moc"

/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <QString>
#include <QWidget>
#include <QLayout>
#include <QRadioButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>

#include <kdeversion.h>
#include <kglobal.h>
#include <klocale.h>
#include <klineedit.h>
#include <kurlrequester.h>
#include <kiconloader.h>
#include <kimageio.h>
#include <kconfig.h>

#include "configuretableviewdialog.h"

ConfigureTableViewWidget::ConfigureTableViewWidget( KABC::AddressBook *ab,
                                                    QWidget *parent )
  : ViewConfigureWidget( ab, parent )
{
  QWidget *page = addPage( i18n( "Look & Feel" ), QString(),
                           KIconLoader::global()->loadIcon( "looknfeel",
                           KIconLoader::Panel ) );

  mPage = new LookAndFeelPage( page );
}

ConfigureTableViewWidget::~ConfigureTableViewWidget()
{
}

void ConfigureTableViewWidget::restoreSettings( const KConfigGroup &config )
{
  ViewConfigureWidget::restoreSettings( config );

  mPage->restoreSettings( config );
}

void ConfigureTableViewWidget::saveSettings( KConfigGroup &config )
{
  ViewConfigureWidget::saveSettings( config );

  mPage->saveSettings( config );
}



LookAndFeelPage::LookAndFeelPage(QWidget *parent)
  : QWidget(parent)
{
  initGUI();

  // Set initial state
  enableBackgroundToggled(mBackgroundBox->isChecked());
}

void LookAndFeelPage::restoreSettings( const KConfigGroup &config )
{
  mAlternateButton->setChecked(config.readEntry("ABackground", true));
  mLineButton->setChecked(config.readEntry("SingleLine", false));
  mToolTipBox->setChecked(config.readEntry("ToolTips", true));

  if (!mAlternateButton->isChecked() && !mLineButton->isChecked())
    mNoneButton->setChecked(true);

  mBackgroundBox->setChecked(config.readEntry("Background", false));
  mBackgroundName->lineEdit()->setText(config.readPathEntry("BackgroundName", QString()));
  mIMPresenceBox->setChecked( config.readEntry( "InstantMessagingPresence", false ) );
}

void LookAndFeelPage::saveSettings( KConfigGroup &config )
{
  config.writeEntry("ABackground", mAlternateButton->isChecked());
  config.writeEntry("SingleLine", mLineButton->isChecked());
  config.writeEntry("ToolTips", mToolTipBox->isChecked());
  config.writeEntry("Background", mBackgroundBox->isChecked());
  config.writePathEntry("BackgroundName", mBackgroundName->lineEdit()->text());
  config.writeEntry( "InstantMessagingPresence", mIMPresenceBox->isChecked() );
}

void LookAndFeelPage::initGUI()
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setSpacing(KDialog::spacingHint());
  layout->setMargin(0);

  QGroupBox *group = new QGroupBox(i18n("Row Separator"));
  layout->addWidget(group);

  mAlternateButton = new QRadioButton(i18n("Alternating backgrounds"));
  mAlternateButton->setObjectName( "mAlternateButton" );
  mLineButton = new QRadioButton(i18n("Single line"));
  mLineButton->setObjectName( "mLineButton" );
  mNoneButton = new QRadioButton(i18n("None"));
  mNoneButton->setObjectName( "mNoneButton" );
  QVBoxLayout *updateLayout = new QVBoxLayout;
  updateLayout->addWidget(mNoneButton);
  updateLayout->addWidget(mLineButton);
  updateLayout->addWidget(mAlternateButton);

  group->setLayout(updateLayout);


  // Background Checkbox/Selector
  QHBoxLayout *backgroundLayout = new QHBoxLayout();
  layout->addLayout(backgroundLayout);

  mBackgroundBox = new QCheckBox(i18n("Enable background image:"), this );
  mBackgroundBox->setObjectName( "mBackgroundBox" );
  connect(mBackgroundBox, SIGNAL(toggled(bool)),
          SLOT(enableBackgroundToggled(bool)));
  backgroundLayout->addWidget(mBackgroundBox);

  mBackgroundName = new KUrlRequester(this);
  mBackgroundName->setObjectName( "mBackgroundName");
  mBackgroundName->setMode(KFile::File | KFile::ExistingOnly |
                           KFile::LocalOnly);
  mBackgroundName->setFilter(KImageIO::pattern(KImageIO::Reading));
  backgroundLayout->addWidget(mBackgroundName);

  // ToolTip Checkbox
  mToolTipBox = new QCheckBox(i18n("Enable contact tooltips"), this );
  mToolTipBox->setObjectName( "mToolTipBox");
  layout->addWidget(mToolTipBox);
  mIMPresenceBox = new QCheckBox( i18n( "Show instant messaging presence" ), this );
  mIMPresenceBox->setObjectName( "mIMPresenceBox" );
  layout->addWidget( mIMPresenceBox );
}

void LookAndFeelPage::enableBackgroundToggled(bool enabled)
{
  mBackgroundName->setEnabled(enabled);
}

#include "configuretableviewdialog.moc"

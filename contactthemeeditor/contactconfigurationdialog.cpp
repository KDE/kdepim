/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "contactconfigurationdialog.h"

#include "configurewidget.h"

#include <KLocale>
#include <KUrlRequester>
#include <KConfig>
#include <KGlobal>
#include <KConfigGroup>

#include <QVBoxLayout>
#include <QLabel>

ContactConfigureDialog::ContactConfigureDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Configure" ) );
    setButtons( Default|Ok|Cancel );
    setButtonFocus( Ok );

    QWidget *w = new QWidget;

    QVBoxLayout *lay = new QVBoxLayout;
    w->setLayout(lay);

    mConfigureWidget = new GrantleeThemeEditor::ConfigureWidget;
    lay->addWidget(mConfigureWidget);

    lay->addStretch();

    setMainWidget(w);
    connect(this, SIGNAL(defaultClicked()), this, SLOT(slotDefaultClicked()));
    connect(this, SIGNAL(okClicked()), this, SLOT(slotOkClicked()));
    readConfig();
}

ContactConfigureDialog::~ContactConfigureDialog()
{
    KSharedConfig::Ptr config = KGlobal::config();

    KConfigGroup group = config->group( QLatin1String("ContactConfigureDialog") );
    group.writeEntry( "Size", size() );
}

void ContactConfigureDialog::slotDefaultClicked()
{
    mConfigureWidget->setDefault();
}

void ContactConfigureDialog::slotOkClicked()
{
    writeConfig();
}

void ContactConfigureDialog::readConfig()
{
    KSharedConfig::Ptr config = KGlobal::config();
    mConfigureWidget->readConfig();

    KConfigGroup group = KConfigGroup( config, "ContactConfigureDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize() );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    } else {
        resize( 600,400);
    }
}

void ContactConfigureDialog::writeConfig()
{
    mConfigureWidget->writeConfig();
}

#include "contactconfigurationdialog.moc"

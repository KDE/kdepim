/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "sieveeditorconfiguredialog.h"
#include "serversievelistwidget.h"
#include "sieveeditorconfigureserverwidget.h"
#include "pimcommon/widgets/configureimmutablewidgetutils.h"
#include "sieveeditorglobalconfig.h"

#include <KLocalizedString>
#include <KSharedConfig>

#include <QTabWidget>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QCheckBox>
#include <QGroupBox>

SieveEditorConfigureDialog::SieveEditorConfigureDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Configure" ) );
    setButtons( Cancel | Ok  );

    QGroupBox *w = new QGroupBox(i18n("Server Sieve"));
    setMainWidget(w);

    QVBoxLayout *layout = new QVBoxLayout;
    w->setLayout(layout);
    mServerWidget = new SieveEditorConfigureServerWidget;
    layout->addWidget(mServerWidget);

    mCloseWallet = new QCheckBox(i18n("Close wallet when close application"));
    layout->addWidget(mCloseWallet);
    loadServerSieveConfig();
    readConfig();
}

SieveEditorConfigureDialog::~SieveEditorConfigureDialog()
{
    writeConfig();
}

void SieveEditorConfigureDialog::loadServerSieveConfig()
{
    mServerWidget->readConfig();
    PimCommon::ConfigureImmutableWidgetUtils::loadWidget(mCloseWallet, SieveEditorGlobalConfig::self()->closeWalletItem());
}

void SieveEditorConfigureDialog::saveServerSieveConfig()
{
    mServerWidget->writeConfig();
    PimCommon::ConfigureImmutableWidgetUtils::saveCheckBox(mCloseWallet, SieveEditorGlobalConfig::self()->closeWalletItem());
    SieveEditorGlobalConfig::self()->writeConfig();
}

void SieveEditorConfigureDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "SieveEditorConfigureDialog" );
    const QSize size = group.readEntry( "Size", QSize(600, 400) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void SieveEditorConfigureDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "SieveEditorConfigureDialog" );
    group.writeEntry( "Size", size() );
    group.sync();
}

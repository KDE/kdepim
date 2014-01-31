/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "autocreatescriptdialog.h"
#include "sievescriptlistbox.h"
#include "sieveconditionwidgetlister.h"
#include "sieveactionwidgetlister.h"
#include "sieveeditorgraphicalmodewidget.h"

#include <KLocalizedString>
#include <KSharedConfig>

using namespace KSieveUi;

AutoCreateScriptDialog::AutoCreateScriptDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Create sieve filter" ) );
    setButtons( Ok|Cancel );
    setButtonFocus( Ok );
    mEditor = new SieveEditorGraphicalModeWidget;
    setMainWidget( mEditor );
    readConfig();
}

AutoCreateScriptDialog::~AutoCreateScriptDialog()
{
    writeConfig();
}

void AutoCreateScriptDialog::loadScript(const QDomDocument &doc, QString &error)
{
    mEditor->loadScript(doc, error);
}

void AutoCreateScriptDialog::setSieveCapabilities(const QStringList &capabilities)
{
    mEditor->setSieveCapabilities(capabilities);
}

QString AutoCreateScriptDialog::script(QString &requires) const
{
    return mEditor->script(requires);
}

void AutoCreateScriptDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "AutoCreateScriptDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize(800,600) );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    }
}

void AutoCreateScriptDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "AutoCreateScriptDialog" );
    group.writeEntry( "Size", size() );
}


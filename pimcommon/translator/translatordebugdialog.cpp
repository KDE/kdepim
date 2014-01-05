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

#include "translatordebugdialog.h"
#include "pimcommon/util/pimutil.h"
#include "pimcommon/texteditor/plaintexteditor/plaintexteditorwidget.h"

#include <KLocalizedString>
#include <KConfigGroup>
#include <KGlobal>
#include <KSharedConfig>

TranslatorDebugDialog::TranslatorDebugDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Translator Debug" ) );
    setButtons( Close|User1 );
    setButtonText(User1, i18n("Save As..."));
    connect(this, SIGNAL(user1Clicked()), this, SLOT(slotSaveAs()));

    mEdit = new PimCommon::PlainTextEditorWidget;
    mEdit->setReadOnly(true);

    setMainWidget( mEdit );
    readConfig();
    enableButton(User1, !mEdit->toPlainText().isEmpty());
}

TranslatorDebugDialog::~TranslatorDebugDialog()
{
    writeConfig();
}

void TranslatorDebugDialog::setDebug(const QString &debugStr)
{
    mEdit->setPlainText(debugStr);
    enableButton(User1, !debugStr.isEmpty());
}

void TranslatorDebugDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "TranslatorDebugDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize(800,600) );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    }
}

void TranslatorDebugDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "TranslatorDebugDialog" );
    group.writeEntry( "Size", size() );
}

void TranslatorDebugDialog::slotSaveAs()
{
    const QString filter = i18n( "all files (*)" );
    PimCommon::Util::saveTextAs(mEdit->toPlainText(), filter, this);
}



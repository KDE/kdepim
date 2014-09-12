/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include "clipboardmenu.h"

#include "kdtoolsglobal.h"
#include "mainwindow.h"

#include <utils/kdsignalblocker.h>

#include <commands/importcertificatefromclipboardcommand.h>
#include <commands/encryptclipboardcommand.h>
#include <commands/signclipboardcommand.h>
#include <commands/decryptverifyclipboardcommand.h>

#include <KLocalizedString>
#include <KActionMenu>

#include <QAction>
#include <QApplication>
#include <QClipboard>

using namespace Kleo;

using namespace Kleo::Commands;

ClipboardMenu::ClipboardMenu(QObject *parent)
    : QObject(parent),
      mWindow(0)
{
    mClipboardMenu = new KActionMenu( i18n("Clipboard" ), this );
    mImportClipboardAction = new QAction( i18n("Certificate Import"), this );
    mEncryptClipboardAction = new QAction( i18n("Encrypt..."), this);
    mSmimeSignClipboardAction = new QAction( i18n("S/MIME-Sign..."), this );
    mOpenPGPSignClipboardAction = new QAction( i18n("OpenPGP-Sign..."), this );
    mDecryptVerifyClipboardAction = new QAction( i18n("Decrypt/Verify..."), this );

    KDAB_SET_OBJECT_NAME( mClipboardMenu );
    KDAB_SET_OBJECT_NAME( mImportClipboardAction );
    KDAB_SET_OBJECT_NAME( mEncryptClipboardAction );
    KDAB_SET_OBJECT_NAME( mSmimeSignClipboardAction );
    KDAB_SET_OBJECT_NAME( mOpenPGPSignClipboardAction );
    KDAB_SET_OBJECT_NAME( mDecryptVerifyClipboardAction );

    connect(mImportClipboardAction, &QAction::triggered, this, &ClipboardMenu::slotImportClipboard);
    connect(mEncryptClipboardAction, &QAction::triggered, this, &ClipboardMenu::slotEncryptClipboard);
    connect(mSmimeSignClipboardAction, &QAction::triggered, this, &ClipboardMenu::slotSMIMESignClipboard);
    connect(mOpenPGPSignClipboardAction, &QAction::triggered, this, &ClipboardMenu::slotOpenPGPSignClipboard);
    connect(mDecryptVerifyClipboardAction, &QAction::triggered, this, &ClipboardMenu::slotDecryptVerifyClipboard);
    mClipboardMenu->addAction( mImportClipboardAction );
    mClipboardMenu->addAction( mEncryptClipboardAction );
    mClipboardMenu->addAction( mSmimeSignClipboardAction );
    mClipboardMenu->addAction( mOpenPGPSignClipboardAction );
    mClipboardMenu->addAction( mDecryptVerifyClipboardAction );
    connect( QApplication::clipboard(), SIGNAL(changed(QClipboard::Mode)), this, SLOT(slotEnableDisableActions()) );
    slotEnableDisableActions();
}

ClipboardMenu::~ClipboardMenu()
{

}

void ClipboardMenu::setMainWindow(MainWindow *window)
{
    mWindow = window;
}

KActionMenu *ClipboardMenu::clipboardMenu() const
{
    return mClipboardMenu;
}

void ClipboardMenu::startCommand( Command * cmd )
{
    Q_ASSERT( cmd );
    cmd->setParent( mWindow );
    cmd->start();
}

void ClipboardMenu::slotImportClipboard()
{
    startCommand( new ImportCertificateFromClipboardCommand( 0 ) );
}

void ClipboardMenu::slotEncryptClipboard()
{
    startCommand( new EncryptClipboardCommand( 0 ) );
}

void ClipboardMenu::slotOpenPGPSignClipboard()
{
    startCommand( new SignClipboardCommand( GpgME::OpenPGP, 0 ) );
}

void ClipboardMenu::slotSMIMESignClipboard()
{
    startCommand( new SignClipboardCommand( GpgME::CMS, 0 ) );
}

void ClipboardMenu::slotDecryptVerifyClipboard()
{
    startCommand( new DecryptVerifyClipboardCommand( 0 ) );
}

void ClipboardMenu::slotEnableDisableActions()
{
    const KDSignalBlocker blocker( QApplication::clipboard() );
    mImportClipboardAction->setEnabled( ImportCertificateFromClipboardCommand::canImportCurrentClipboard() );
    mEncryptClipboardAction->setEnabled( EncryptClipboardCommand::canEncryptCurrentClipboard() );
    mOpenPGPSignClipboardAction->setEnabled( SignClipboardCommand::canSignCurrentClipboard() );
    mSmimeSignClipboardAction->setEnabled( SignClipboardCommand::canSignCurrentClipboard() );
    mDecryptVerifyClipboardAction->setEnabled( DecryptVerifyClipboardCommand::canDecryptVerifyCurrentClipboard() );
}

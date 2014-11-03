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

#ifndef CLIPBOARDMENU_H
#define CLIPBOARDMENU_H

#include <QObject>
class KActionMenu;
class QAction;
class MainWindow;
namespace Kleo
{
class Command;
}
class ClipboardMenu : public QObject
{
    Q_OBJECT
public:
    explicit ClipboardMenu(QObject *parent = 0);
    ~ClipboardMenu();

    void setMainWindow(MainWindow *window);

    KActionMenu *clipboardMenu() const;

private Q_SLOTS:
    void slotImportClipboard();
    void slotEncryptClipboard();
    void slotOpenPGPSignClipboard();
    void slotSMIMESignClipboard();
    void slotDecryptVerifyClipboard();
    void slotEnableDisableActions();

private:
    void startCommand(Kleo::Command *cmd);

    KActionMenu *mClipboardMenu;
    QAction *mImportClipboardAction;
    QAction *mEncryptClipboardAction;
    QAction *mSmimeSignClipboardAction;
    QAction *mOpenPGPSignClipboardAction;
    QAction *mDecryptVerifyClipboardAction;
    MainWindow *mWindow;
};

#endif // CLIPBOARDMENU_H

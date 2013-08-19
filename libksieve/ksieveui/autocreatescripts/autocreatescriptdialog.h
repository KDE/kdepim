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

#ifndef AUTOCREATESCRIPTDIALOG_H
#define AUTOCREATESCRIPTDIALOG_H

#include "ksieveui_export.h"

#include <KDialog>

class QStackedWidget;
class QSplitter;

namespace KSieveUi {
class SieveScriptListBox;
class KSIEVEUI_EXPORT AutoCreateScriptDialog : public KDialog
{
    Q_OBJECT
public:
    explicit AutoCreateScriptDialog(QWidget *parent = 0);
    ~AutoCreateScriptDialog();
    QString script(QString &requires) const;

    static void setSieveCapabilities( const QStringList &capabilities );
    static QStringList sieveCapabilities();

private Q_SLOTS:
    void slotAddScriptPage(QWidget *page);
    void slotRemoveScriptPage(QWidget *page);
    void slotActivateScriptPage(QWidget *page);

private:
    void readConfig();
    void writeConfig();

private:
    static QStringList sCapabilities;
    SieveScriptListBox *mSieveScript;
    QStackedWidget *mStackWidget;
    QSplitter *mSplitter;
};
}

#endif // AUTOCREATESCRIPTDIALOG_H

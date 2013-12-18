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

#ifndef MULTIIMAPVACATIONDIALOG_H
#define MULTIIMAPVACATIONDIALOG_H

#include <KDialog>

#include "ksieveui_export.h"

class QTabWidget;
class QStackedWidget;
namespace KSieveUi {
class VacationCreateScriptJob;
class KSIEVEUI_EXPORT MultiImapVacationDialog : public KDialog
{
    Q_OBJECT
public:
    explicit MultiImapVacationDialog(QWidget *parent=0);
    ~MultiImapVacationDialog();

    QList<VacationCreateScriptJob *> listCreateJob() const;

    void switchToServerNamePage(const QString &serverName);

private slots:
    void slotOkClicked();    
    void slotDefaultClicked();

private:
    void createPage(const QString &serverName, const KUrl &url);
    void init();
    void readConfig();
    void writeConfig();
    QList<VacationCreateScriptJob *> mListCreateJob;
    QTabWidget *mTabWidget;
    QStackedWidget *mStackedWidget;
};
}

#endif // MULTIIMAPVACATIONDIALOG_H

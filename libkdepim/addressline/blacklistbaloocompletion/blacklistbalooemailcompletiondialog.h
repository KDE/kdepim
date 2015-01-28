/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#ifndef BLACKLISTBALOOEMAILCOMPLETIONDIALOG_H
#define BLACKLISTBALOOEMAILCOMPLETIONDIALOG_H

#include <KDialog>
#include "kdepim_export.h"
class KLineEdit;
class QPushButton;
namespace KPIM {
class BlackListBalooEmailList;
class KDEPIM_EXPORT BlackListBalooEmailCompletionDialog : public KDialog
{
    Q_OBJECT
public:
    explicit BlackListBalooEmailCompletionDialog(QWidget *parent=0);
    ~BlackListBalooEmailCompletionDialog();

    void setEmailBlackList(const QStringList &list);

private Q_SLOTS:
    void slotSave();
    void slotSearch();

    void slotSearchLineEditChanged(const QString &text);
    void slotUnselectEmails();
    void slotSelectEmails();
private:
    void writeConfig();
    void readConfig();
    KLineEdit *mSearchLineEdit;
    BlackListBalooEmailList *mEmailList;
    QPushButton *mSearchButton;
};
}

#endif // BLACKLISTBALOOEMAILCOMPLETIONDIALOG_H

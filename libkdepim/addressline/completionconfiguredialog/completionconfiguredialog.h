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

#ifndef COMPLETIONCONFIGUREDIALOG_H
#define COMPLETIONCONFIGUREDIALOG_H

#include "kdepim_export.h"

#include <QDialog>
class QTabWidget;
namespace KLDAP {
class LdapClientSearch;
}
namespace KPIM {
class CompletionOrderWidget;
class BlackListBalooEmailCompletionWidget;
class RecentAddressWidget;
class KDEPIM_EXPORT CompletionConfigureDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CompletionConfigureDialog(QWidget *parent = 0);
    ~CompletionConfigureDialog();

    void load();
    void setEmailBlackList(const QStringList &lst);
    void setLdapClientSearch(KLDAP::LdapClientSearch *ldapSearch);
    void setRecentAddresses(const QStringList &lst);
private slots:
    void slotSave();

private:
    void readConfig();
    void writeConfig();
    QTabWidget *mTabWidget;
    KPIM::CompletionOrderWidget *mCompletionOrderWidget;
    KPIM::BlackListBalooEmailCompletionWidget *mBlackListBalooWidget;
    KPIM::RecentAddressWidget *mRecentaddressWidget;
};
}

#endif // COMPLETIONCONFIGUREDIALOG_H

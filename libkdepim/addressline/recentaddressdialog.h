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

#ifndef RECENTADDRESSDIALOG_H
#define RECENTADDRESSDIALOG_H

#include "kdepim_export.h"
#include <kabc/addressee.h>
#include <KDialog>
#include <QStringList>
class KConfig;
class KPushButton;
class QListWidget;
class KLineEdit;


namespace KPIM {

class KDEPIM_EXPORT RecentAddressDialog : public KDialog
{
    Q_OBJECT
public:
    explicit RecentAddressDialog( QWidget *parent );
    ~RecentAddressDialog();

    void setAddresses( const QStringList &addrs );
    QStringList addresses() const;
    void addAddresses(KConfig *config);
    bool wasChanged() const;
private slots:
    void slotAddItem();
    void slotRemoveItem();
    void slotSelectionChanged();
    void slotTypedSomething(const QString&);

protected:
    void updateButtonState();
    bool eventFilter( QObject* o, QEvent* e );

private:
    void readConfig();
    void writeConfig();
    KPushButton* mNewButton, *mRemoveButton;
    QListWidget *mListView;
    KLineEdit *mLineEdit;
    bool mDirty;
};
}

#endif // RECENTADDRESSDIALOG_H

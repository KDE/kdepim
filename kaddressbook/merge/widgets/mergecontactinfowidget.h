/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#ifndef MERGECONTACTINFOWIDGET_H
#define MERGECONTACTINFOWIDGET_H

#include <QWidget>

#include "kaddressbook_export.h"

#include <Akonadi/Item>
class QLabel;
namespace KAddressBookGrantlee {
class GrantleeContactViewer;
}

class QStackedWidget;
namespace KABMergeContacts {
class MergeContactInfoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MergeContactInfoWidget(QWidget *parent=0);
    ~MergeContactInfoWidget();

public Q_SLOTS:
    void setContact(const Akonadi::Item &item);

private:
    QStackedWidget *mStackWidget;
    QLabel *mNoContactSelected;
    KAddressBookGrantlee::GrantleeContactViewer *mContactViewer;
};
}

#endif // MERGECONTACTINFOWIDGET_H

/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#ifndef ACLLISTVIEW_H
#define ACLLISTVIEW_H

#include <QListView>
#include "pimcommon_export.h"
namespace PimCommon
{
class PIMCOMMON_EXPORT AclListView : public QListView
{
    Q_OBJECT
public:
    explicit AclListView(QWidget *parent = Q_NULLPTR);
    ~AclListView();

public Q_SLOTS:
    void slotCollectionCanBeAdministrated(bool b);

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    void generalPaletteChanged();
    QColor mTextColor;
    bool mCanBeAdministrated;
};
}
#endif // ACLLISTVIEW_H

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

#ifndef INVALIDFILTERLISTITEMDELEGATE_H
#define INVALIDFILTERLISTITEMDELEGATE_H

#include <KWidgetItemDelegate>

namespace MailCommon {
class InvalidFilterListItemDelegate : public KWidgetItemDelegate
{
    Q_OBJECT
public:
    explicit InvalidFilterListItemDelegate(QAbstractItemView* itemView, QObject* parent = 0);
    virtual ~InvalidFilterListItemDelegate();

    virtual QSize sizeHint(const QStyleOptionViewItem &option,
                           const QModelIndex &index) const;

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option,
                       const QModelIndex& index) const;

    virtual QList<QWidget*> createItemWidgets() const;

    virtual void updateItemWidgets(const QList<QWidget*> widgets,
                                   const QStyleOptionViewItem& option,
                                   const QPersistentModelIndex& index) const;
private Q_SLOTS:
    void slotShowDetails();
Q_SIGNALS:
    void showDetails(const QString &details);
};
}
#endif // INVALIDFILTERLISTITEMDELEGATE_H
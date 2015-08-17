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
#include "invalidfilterlistitemdelegate.h"
#include "invalidfilterlistmodel.h"

#include <QIcon>
#include <QAbstractItemView>
#include <QPainter>
#include <QPushButton>
#include <QToolButton>
#include <qlabel.h>

using namespace MailCommon;

InvalidFilterListItemDelegate::InvalidFilterListItemDelegate(QAbstractItemView *itemView, QObject *parent) :
    KWidgetItemDelegate(itemView, parent)
{
}

InvalidFilterListItemDelegate::~InvalidFilterListItemDelegate()
{
}

QSize InvalidFilterListItemDelegate::sizeHint(const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    Q_UNUSED(index);

    const QStyle *style = itemView()->style();
    const int buttonHeight = style->pixelMetric(QStyle::PM_ButtonMargin) * 2 +
                             style->pixelMetric(QStyle::PM_ButtonIconSize);
    const int fontHeight = option.fontMetrics.height();
    return QSize(100, qMax(buttonHeight, fontHeight));
}

void InvalidFilterListItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    Q_UNUSED(index);
    painter->save();

    itemView()->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.highlightedText().color());
    }

    painter->restore();
}

QList<QWidget *> InvalidFilterListItemDelegate::createItemWidgets(const QModelIndex &) const
{
    QLabel *label = new QLabel();
    QToolButton *showInformationToolButton = new QToolButton();
    connect(showInformationToolButton, &QAbstractButton::clicked, this, &InvalidFilterListItemDelegate::slotShowDetails);
    return QList<QWidget *>() << label << showInformationToolButton;
}

void InvalidFilterListItemDelegate::updateItemWidgets(const QList<QWidget *> widgets,
        const QStyleOptionViewItem &option,
        const QPersistentModelIndex &index) const
{
    QLabel *label = static_cast<QLabel *>(widgets[0]);
    const QAbstractItemModel *model = index.model();
    label->setText(model->data(index).toString());
    const QString details = model->data(index, InvalidFilterListModel::InformationRole).toString();

    QToolButton *showInformationToolButton = static_cast<QToolButton *>(widgets[1]);
    showInformationToolButton->setEnabled(!details.isEmpty());
    const int itemHeight = sizeHint(option, index).height();
    showInformationToolButton->setIcon(QIcon::fromTheme(QStringLiteral("help-hint")));
    showInformationToolButton->resize(showInformationToolButton->sizeHint());
    showInformationToolButton->move(option.rect.right() - showInformationToolButton->width(),
                                    (itemHeight - showInformationToolButton->height()) / 2);

    int labelWidth = option.rect.width();
    labelWidth -= showInformationToolButton->sizeHint().width();
    label->resize(labelWidth, label->sizeHint().height());
    label->move(0, (itemHeight - label->height()) / 2);
}

void InvalidFilterListItemDelegate::slotShowDetails()
{
    const QAbstractItemModel *model = focusedIndex().model();

    const QString information = model->data(focusedIndex(), InvalidFilterListModel::InformationRole).toString();
    if (!information.isEmpty()) {
        Q_EMIT showDetails(information);
    }
}


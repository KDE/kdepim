/*
 * This file is part of KMail.
 * Copyright (c) 2011-2016 Laurent Montel <montel@kde.org>
 *
 * Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>
 *
 * Parts based on KMail code by:
 * Copyright (c) 2003 Ingo Kloecker <kloecker@kde.org>
 * Copyright (c) 2007 Thomas McGuire <Thomas.McGuire@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "attachmentview.h"

#include <MessageComposer/AttachmentModel>
#include "kmkernel.h"

#include <QContextMenuEvent>
#include <QHeaderView>
#include <QKeyEvent>
#include <QSortFilterProxyModel>
#include <QToolButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QDrag>

#include <KConfigGroup>
#include <QIcon>
#include <KLocalizedString>

#include <MessageCore/AttachmentPart>
#include <KFormat>
using MessageCore::AttachmentPart;

using namespace KMail;

class KMail::AttachmentView::Private
{
public:
    Private(AttachmentView *qq)
        : model(Q_NULLPTR), q(qq)
    {
        widget = new QWidget();
        QHBoxLayout *lay = new QHBoxLayout;
        lay->setMargin(0);
        widget->setLayout(lay);
        toolButton = new QToolButton;
        connect(toolButton, &QAbstractButton::toggled, q, &AttachmentView::slotShowHideAttchementList);
        toolButton->setIcon(QIcon::fromTheme(QStringLiteral("mail-attachment")));
        toolButton->setAutoRaise(true);
        toolButton->setCheckable(true);
        lay->addWidget(toolButton);
        infoAttachment = new QLabel;
        infoAttachment->setMargin(0);
        lay->addWidget(infoAttachment);
    }

    MessageComposer::AttachmentModel *model;
    QToolButton *toolButton;
    QLabel *infoAttachment;
    QWidget *widget;
    AttachmentView *q;
};

AttachmentView::AttachmentView(MessageComposer::AttachmentModel *model, QWidget *parent)
    : QTreeView(parent)
    , d(new Private(this))
{
    d->model = model;
    connect(model, &MessageComposer::AttachmentModel::encryptEnabled, this, &AttachmentView::setEncryptEnabled);
    connect(model, &MessageComposer::AttachmentModel::signEnabled, this, &AttachmentView::setSignEnabled);

    QSortFilterProxyModel *sortModel = new QSortFilterProxyModel(this);
    sortModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    sortModel->setSourceModel(model);
    setModel(sortModel);
    connect(model, &MessageComposer::AttachmentModel::rowsInserted, this, &AttachmentView::hideIfEmpty);
    connect(model, &MessageComposer::AttachmentModel::rowsRemoved, this, &AttachmentView::hideIfEmpty);
    connect(model, &MessageComposer::AttachmentModel::rowsRemoved, this, &AttachmentView::selectNewAttachment);
    connect(model, &MessageComposer::AttachmentModel::dataChanged, this, &AttachmentView::updateAttachmentLabel);

    setRootIsDecorated(false);
    setUniformRowHeights(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDropIndicatorShown(false);
    setSortingEnabled(true);

    header()->setResizeMode(QHeaderView::Interactive);
    header()->setStretchLastSection(false);
    restoreHeaderState();
    setColumnWidth(0, 200);
}

AttachmentView::~AttachmentView()
{
    saveHeaderState();
    delete d;
}

void AttachmentView::restoreHeaderState()
{
    KConfigGroup grp(KMKernel::self()->config(), "AttachmentView");
    header()->restoreState(grp.readEntry("State", QByteArray()));
}

void AttachmentView::saveHeaderState()
{
    KConfigGroup grp(KMKernel::self()->config(), "AttachmentView");
    grp.writeEntry("State", header()->saveState());
    grp.sync();
}

void AttachmentView::contextMenuEvent(QContextMenuEvent *event)
{
    Q_UNUSED(event);
    Q_EMIT contextMenuRequested();
}

void AttachmentView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete) {
        // Indexes are based on row numbers, and row numbers change when items are deleted.
        // Therefore, first we need to make a list of AttachmentParts to delete.
        AttachmentPart::List toRemove;
        const QModelIndexList selectedIndexes = selectionModel()->selectedRows();
        toRemove.reserve(selectedIndexes.count());
        foreach (const QModelIndex &index, selectedIndexes) {
            AttachmentPart::Ptr part = model()->data(
                                           index, MessageComposer::AttachmentModel::AttachmentPartRole).value<AttachmentPart::Ptr>();
            toRemove.append(part);
        }
        foreach (const AttachmentPart::Ptr &part, toRemove) {
            d->model->removeAttachment(part);
        }
    }
}

void AttachmentView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->source() == this) {
        // Ignore drags from ourselves.
        event->ignore();
    } else {
        QTreeView::dragEnterEvent(event);
    }
}

void AttachmentView::setEncryptEnabled(bool enabled)
{
    setColumnHidden(MessageComposer::AttachmentModel::EncryptColumn, !enabled);
}

void AttachmentView::setSignEnabled(bool enabled)
{
    setColumnHidden(MessageComposer::AttachmentModel::SignColumn, !enabled);
}

void AttachmentView::hideIfEmpty()
{
    const bool needToShowIt = (model()->rowCount() > 0);
    setVisible(needToShowIt);
    d->toolButton->setChecked(needToShowIt);
    widget()->setVisible(needToShowIt);
    if (needToShowIt) {
        updateAttachmentLabel();
    } else {
        d->infoAttachment->clear();
    }
    Q_EMIT modified(true);
}

void AttachmentView::updateAttachmentLabel()
{
    MessageCore::AttachmentPart::List list = d->model->attachments();
    qint64 size = 0;
    Q_FOREACH (const MessageCore::AttachmentPart::Ptr &part, list) {
        size += part->size();
    }
    d->infoAttachment->setText(i18np("1 attachment (%2)", "%1 attachments (%2)", model()->rowCount(), KFormat().formatByteSize(qMax(0LL, size))));
}

void AttachmentView::selectNewAttachment()
{
    if (selectionModel()->selectedRows().isEmpty()) {
        selectionModel()->select(selectionModel()->currentIndex(),
                                 QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
}

void AttachmentView::startDrag(Qt::DropActions supportedActions)
{
    Q_UNUSED(supportedActions);

    const QModelIndexList selection = selectionModel()->selectedRows();
    if (!selection.isEmpty()) {
        QMimeData *mimeData = model()->mimeData(selection);
        QDrag *drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->exec(Qt::CopyAction);
    }
}

QWidget *AttachmentView::widget() const
{
    return d->widget;
}

void AttachmentView::slotShowHideAttchementList(bool show)
{
    setVisible(show);
    if (show) {
        d->toolButton->setToolTip(i18n("Hide attachment list"));
    } else {
        d->toolButton->setToolTip(i18n("Show attachment list"));
    }
}


/*
 * Copyright (c) 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * As a special exception, permission is given to link this program
 * with any edition of Qt, and distribute the resulting executable,
 * without including the source code for Qt in the source distribution.
 */

#include "attendeelineeditdelegate.h"

#include "attendeeline.h"

#include <KLocalizedString>

#include <QAbstractItemView>
#include <QHelpEvent>
#include <QToolTip>
#include <QWhatsThis>

using namespace IncidenceEditorNG;

AttendeeLineEditDelegate::AttendeeLineEditDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    mToolTip = i18nc("@info:tooltip",
                     "Enter the name or email address of the attendee.");
    mWhatsThis = i18nc("@info:whatsthis",
                       "The email address or name of the attendee. An invitation "
                       "can be sent to the user if an email address is provided.");
}

QWidget *AttendeeLineEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    AttendeeLineEdit *editor = new AttendeeLineEdit(parent);
    connect(editor, &AttendeeLineEdit::leftPressed, this, &AttendeeLineEditDelegate::leftPressed);
    connect(editor, &AttendeeLineEdit::rightPressed, this, &AttendeeLineEditDelegate::rightPressed);
    editor->setToolTip(mToolTip);
    editor->setWhatsThis(mWhatsThis);
    editor->setCompletionMode(mCompletionMode);
    editor->setClearButtonShown(true);

    return editor;
}

void AttendeeLineEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    AttendeeLineEdit *lineedit = static_cast<AttendeeLineEdit *>(editor);
    lineedit->setText(index.model()->data(index, Qt::EditRole).toString());
}

void AttendeeLineEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    AttendeeLineEdit *lineedit = static_cast<AttendeeLineEdit *>(editor);
    model->setData(index, lineedit->text(), Qt::EditRole);
}

void AttendeeLineEditDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}

void AttendeeLineEditDelegate::leftPressed()
{
    Q_EMIT closeEditor(static_cast<QWidget *>(QObject::sender()), QAbstractItemDelegate::EditPreviousItem);
}

void AttendeeLineEditDelegate::rightPressed()
{
    Q_EMIT closeEditor(static_cast<QWidget *>(QObject::sender()), QAbstractItemDelegate::EditNextItem);
}

void AttendeeLineEditDelegate::setCompletionMode(KCompletion::CompletionMode mode)
{
    mCompletionMode = mode;
}

bool AttendeeLineEditDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view,
        const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (!event || !view) {
        return false;
    }
    switch (event->type()) {
#ifndef QT_NO_TOOLTIP
    case QEvent::ToolTip: {
        QHelpEvent *he = static_cast<QHelpEvent *>(event);
        QToolTip::showText(he->globalPos(), mToolTip, view);
        return true;
        break;
    }
#endif
#ifndef QT_NO_WHATSTHIS
    case QEvent::QueryWhatsThis:
        return true;
        break;
    case QEvent::WhatsThis: {
        QHelpEvent *he = static_cast<QHelpEvent *>(event);
        QWhatsThis::showText(he->globalPos(), mWhatsThis, view);
        return true;
        break;
    }
#endif
    default:
        break;
    }
    return QStyledItemDelegate::helpEvent(event, view, option, index);
}

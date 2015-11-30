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

#include "attendeecomboboxdelegate.h"

#include "attendeeline.h"

#include <KLocalizedString>

#include <QAbstractItemView>
#include <QApplication>
#include <QHelpEvent>
#include <QMenu>
#include <QToolTip>
#include <QWhatsThis>

using namespace IncidenceEditorNG;

AttendeeComboBoxDelegate::AttendeeComboBoxDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
    , mStandardIndex(0)
{
    connect(this, &AttendeeComboBoxDelegate::closeEditor, this, &AttendeeComboBoxDelegate::doCloseEditor);
}

void AttendeeComboBoxDelegate::addItem(const QIcon &icon, const QString &text)
{
    QPair<QIcon, QString> pair;
    pair.first = icon;
    pair.second = text;
    mEntries << pair;
}

void AttendeeComboBoxDelegate::clear()
{
    mEntries.clear();
}

void AttendeeComboBoxDelegate::setToolTip(const QString &tT)
{
    mToolTip = tT;
}

void AttendeeComboBoxDelegate::setWhatsThis(const QString &wT)
{
    mWhatsThis = wT;
}

void AttendeeComboBoxDelegate::setStandardIndex(int index)
{
    mStandardIndex = index;
}

QWidget *AttendeeComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex &/* index */) const
{
    AttendeeComboBox *editor = new AttendeeComboBox(parent);
    QPair<QIcon, QString> pair;

    foreach (pair, mEntries) {
        editor->addItem(pair.first, pair.second);
    }

    connect(editor, &AttendeeComboBox::leftPressed, this, &AttendeeComboBoxDelegate::leftPressed);
    connect(editor, &AttendeeComboBox::rightPressed, this, &AttendeeComboBoxDelegate::rightPressed);

    editor->setPopupMode(QToolButton::MenuButtonPopup);
    editor->setToolTip(mToolTip);
    editor->setWhatsThis(mWhatsThis);
    return editor;
}

void AttendeeComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    AttendeeComboBox *comboBox = static_cast<AttendeeComboBox *>(editor);
    int value = index.model()->data(index, Qt::EditRole).toUInt();
    if (value >= mEntries.count()) {
        value = mStandardIndex;
    }
    comboBox->setCurrentIndex(value);
}

void AttendeeComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    AttendeeComboBox *comboBox = static_cast<AttendeeComboBox *>(editor);
    model->setData(index, comboBox->currentIndex(), Qt::EditRole);
    comboBox->menu()->close();
}

void AttendeeComboBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

void AttendeeComboBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionButton myOption;;

    int value = index.model()->data(index).toUInt();
    if (value >= mEntries.count()) {
        value = mStandardIndex;
    }

    myOption.rect = option.rect;
    myOption.state = option.state;
    myOption.icon = mEntries[value].first;

    QApplication::style()->drawControl(QStyle::CE_PushButton, &myOption, painter);
}

bool AttendeeComboBoxDelegate::eventFilter(QObject *editor, QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        AttendeeComboBox *comboBox = static_cast<AttendeeComboBox *>(editor);
        comboBox->showMenu();
        return editor->eventFilter(editor, event);
    }

    return QStyledItemDelegate::eventFilter(editor, event);
}

void AttendeeComboBoxDelegate::doCloseEditor(QWidget *editor)
{
    AttendeeComboBox *comboBox = static_cast<AttendeeComboBox *>(editor);
    comboBox->menu()->close();
}

void AttendeeComboBoxDelegate::leftPressed()
{
    Q_EMIT closeEditor(static_cast<QWidget *>(QObject::sender()), QAbstractItemDelegate::EditPreviousItem);
}

void AttendeeComboBoxDelegate::rightPressed()
{
    Q_EMIT closeEditor(static_cast<QWidget *>(QObject::sender()), QAbstractItemDelegate::EditNextItem);
}

bool AttendeeComboBoxDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view,
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

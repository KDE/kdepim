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

#ifndef INCIDENCEEDITOR_ATTENDEECOMBOBOXDELEGATE_H
#define INCIDENCEEDITOR_ATTENDEECOMBOBOXDELEGATE_H

#include <QStyledItemDelegate>
#include <QModelIndex>
#include <QIcon>
#include <QString>

namespace IncidenceEditorNG
{

/**
 * class to show a Icon and Text for an Attendee
 * you have to set the Items via addItem to have a list to choose from.
 * saves the option as int in the model
 */
class AttendeeComboBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit AttendeeComboBoxDelegate(QObject *parent = Q_NULLPTR);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
    void setEditorData(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const Q_DECL_OVERRIDE;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;

    bool eventFilter(QObject *editor, QEvent *event) Q_DECL_OVERRIDE;

    virtual void addItem(const QIcon &, const QString &);
    virtual void clear();

    virtual void setToolTip(const QString &);
    virtual void setWhatsThis(const QString &);

    /** choose this index, if the item in the model is unknown
     */
    void setStandardIndex(int);

public Q_SLOTS:
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void doCloseEditor(QWidget *editor);
    void rightPressed();
    void leftPressed();

private:
    /** all entries to choose from */
    QVector<QPair<QIcon, QString> > mEntries;
    QString mToolTip;
    QString mWhatsThis;
    /**fallback index */
    int mStandardIndex;
};

}

#endif

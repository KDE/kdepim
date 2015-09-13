/*
  This file is part of KOrganizer.

  Copyright (c) 2008 Thomas Thrainer <tom_t@gmx.at>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef CALENDARVIEWS_TODOVIEWVIEW_H
#define CALENDARVIEWS_TODOVIEWVIEW_H

#include <QTreeView>
#include <QTimer>

class QMenu;

class TodoViewView : public QTreeView
{
    Q_OBJECT

public:
    explicit TodoViewView(QWidget *parent = Q_NULLPTR);

    bool isEditing(const QModelIndex &index) const;

    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

protected:
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *) Q_DECL_OVERRIDE;

private:
    QModelIndex getNextEditableIndex(const QModelIndex &cur, int inc);

    QMenu *mHeaderPopup;
    QList<QAction *> mColumnActions;
    QTimer mExpandTimer;
    bool mIgnoreNextMouseRelease;

Q_SIGNALS:
    void visibleColumnCountChanged();

private Q_SLOTS:
    void toggleColumnHidden(QAction *action);
    void expandParent();
};

#endif

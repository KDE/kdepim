/*
    This file is part of Akonadi.

    Copyright (c) 2010 Stephen Kelly <steveire@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

// READ THE README FILE

#ifndef NOTEVIEWER_H
#define NOTEVIEWER_H

#include <QWidget>
#include <QPersistentModelIndex>

class QLineEdit;
class QTextEdit;

class KJob;

class NoteViewer : public QWidget
{
    Q_OBJECT
public:
    NoteViewer(QWidget *parent = 0, Qt::WindowFlags f = 0);

    void setIndex(const QPersistentModelIndex &index);

protected:
    virtual bool eventFilter(QObject *watched, QEvent *event);

private slots:
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void modifyDone(KJob *job);

private:
    void populateWidget(const QModelIndex &index);
private:
    QPersistentModelIndex m_persistentIndex;
    QLineEdit *m_titleEdit;
    QTextEdit *m_contentEdit;

};

#endif


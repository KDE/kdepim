/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef ATTENDEESELECTOR_H_H
#define ATTENDEESELECTOR_H_H

#include <QDialog>
#include <QPushButton>
#include "ui_attendeeselector.h"

/**
  Dialog to select a set off attendees.
*/
class AttendeeSelector : public QDialog
{
    Q_OBJECT
public:
    explicit AttendeeSelector(QWidget *parent = Q_NULLPTR);

    QStringList attendees() const;

private Q_SLOTS:
    void addClicked();
    void removeClicked();
    void textChanged(const QString &text);
    void selectionChanged();

private:
    Ui::AttendeeSelectorWidget ui;
    QPushButton *mOkButton;
};

#endif

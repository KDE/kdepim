/*
    This file is part of Akonadi.

    Copyright (c) 2011 Stephen Kelly <steveire@gmail.com>

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

#ifndef TRIPCOMPONENTFACTORY_H
#define TRIPCOMPONENTFACTORY_H

class QObject;
class QAbstractItemModel;

namespace Akonadi
{
class ChangeRecorder;
}

class TripComponentFactory
{
public:

    Akonadi::ChangeRecorder *createMailChangeRecorder(QObject *parent);
    Akonadi::ChangeRecorder *createTodoChangeRecorder(QObject *parent);
    Akonadi::ChangeRecorder *createNotesChangeRecorder(QObject *parent);

    QAbstractItemModel *createMailModel(Akonadi::ChangeRecorder *changeRecorder);
    QAbstractItemModel *createTodoModel(Akonadi::ChangeRecorder *changeRecorder);
    QAbstractItemModel *createNotesModel(Akonadi::ChangeRecorder *changeRecorder);

};

#endif
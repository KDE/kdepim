/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#ifndef CREATETODOJOB_H
#define CREATETODOJOB_H
#include <KJob>
#include <AkonadiCore/Item>
#include <AkonadiCore/Collection>
#include <KCalCore/Todo>

#include <QObject>
namespace MessageViewer
{
class CreateTodoJob : public KJob
{
    Q_OBJECT
public:
    explicit CreateTodoJob(const KCalCore::Todo::Ptr &todoPtr, const Akonadi::Collection &collection, const Akonadi::Item &item, QObject *parent = 0);
    ~CreateTodoJob();

    void start() Q_DECL_OVERRIDE;

private slots:
    void slotFetchDone(KJob *job);
    void slotCreateNewTodo(KJob *job);

private:
    void createTodo();
    Akonadi::Item mItem;
    Akonadi::Collection mCollection;
    KCalCore::Todo::Ptr mTodoPtr;
};
}

#endif // CREATETODOJOB_H

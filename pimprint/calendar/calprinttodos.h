/*
 *  This file is part of the PimPrint library.
 *
 *  Copyright (C) 2013 Allen Winter <winter@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to the
 *  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef PIMPRINT_CALPRINTTODOS_H
#define PIMPRINT_CALPRINTTODOS_H

#include "calprintbase.h"

template<class T >
class to;
namespace PimPrint
{

namespace Calendar
{

class PIMPRINT_CALENDAR_EXPORT CalPrintTodos : public CalPrintBase
{
    Q_ENUMS(TodoTypes)

    Q_PROPERTY(QDate startDate
               READ startDate WRITE setStartDate)

    Q_PROPERTY(QDate endDate
               READ endDate WRITE setEndDate)

    Q_PROPERTY(TodoTypes todoType
               READ todoType WRITE setTodoType)

    Q_PROPERTY(QString headerText
               READ headerText WRITE setHeaderText)

    Q_PROPERTY(KCalCore::SortDirection
               READ sortDirection WRITE setSortDirection)

    Q_PROPERTY(KCalCore::TodoSortField
               READ sortField WRITE setSortField)

    /**
     * Internal class representing the start of a todo.
     */
    class TodoParentStart;

public:
    explicit CalPrintTodos(QPrinter *printer);

    virtual ~CalPrintTodos();

    void print(QPainter &p);

    /**
     * Various types of to-dos that can be printed.
     */
    enum TodoTypes {
        AllTodos,         //< any kind
        Completed,        //< completed TODO: IMPLEMENT
        InProgressTodos,  //< not completed, but started and not past the due date
        NotStarted,       //< not started yet (no start date and 0% completed) TODO: IMPLEMENT
        OpenEnded,        //< no due date TODO: IMPLEMENT
        OverDue,          //< overdue (not completed yet and past the due date) TODO: IMPLEMENT
        DueDateRangeTodos //< due date is with the specified start/due range
    };

    /**
     * Sets the printout starting date.
     * @param dt is the starting date to print.
     * @see startDate(), setEndDate()
     */
    void setStartDate(const QDate &date);

    /**
     * Returns the current print starting date.
     * @see setStartDate(), endDate()
     */
    QDate startDate() const;

    /**
     * Sets the printout ending date.
     * @param dt is the ending date to print.
     * @see endDate(), setStartDate()
     */
    void setEndDate(const QDate &date);

    /**
     * Returns the current print ending date.
     * @see setEndDate(), startDate()
     */
    QDate endDate() const;

    /**
     * Sets the type of to-dos to print.
     * @param todoType is a TodoTypes enum that specifies the types of to-dos to print.
     * @see todoType()
     */
    void setTodoType(TodoTypes todoType);

    /**
     * Returns the current TodoTypes enum.
     * @see setTodoType()
     */
    TodoTypes todoType() const;

    /**
     * Sets the text for the print header.
     * @param text is the string for the print header.
     * @see headerText()
     */
    void setHeaderText(const QString &text);

    /**
     * Returns the current header text string.
     * @see setHeaderText()
     */
    QString headerText() const;

    /**
     * Sets the sorting to-do property.
     * @param sortField sort on this to-do property when printing.
     * @see sortField(), sortDirection()
     */
    void setSortField(const KCalCore::TodoSortField &sortField);

    /**
     * Returns the current sorting field.
     * @see setSortField(), setSortDirection()
     */
    KCalCore::TodoSortField sortField() const;

    /**
     * Sets the sorting direction.
     * @param sortDirection sort in this direction when printing.
     * @see sortDirection(), sortField()
     */
    void setSortDirection(const KCalCore::SortDirection &sortDirection);

    /**
     * Returns the current sorting direction.
     * @see setSortDirection(), setSortField()
     */
    KCalCore::SortDirection sortDirection() const;

private:
    //TODO: move to dpointer
    void drawTodoLines(QPainter &p, const QString &description,
                       int x, int &y,
                       int width, int pageHeight,
                       bool richTextDescription,
                       QList<TodoParentStart *> &startPoints);

    void drawTodo(int &count, const KCalCore::Todo::Ptr &todo, QPainter &p,
                  int posPriority, int posSummary,
                  int posDueDt, int posPercentComplete,
                  int level, int x, int &y, int width,
                  int pageHeight, const KCalCore::Todo::List &todoList,
                  TodoParentStart *r);

    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

}

#endif

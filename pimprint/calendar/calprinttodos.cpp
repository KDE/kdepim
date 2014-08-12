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

#include "calprinttodos.h"

#include <KCalCore/Todo>


#include <KLocale>
#include <KWordWrap>

using namespace PimPrint::Calendar;

//@cond PRIVATE
// The Todo positioning structure
class PimPrint::Calendar::CalPrintTodos::TodoParentStart
{
public:
    TodoParentStart(const QRect &pt = QRect(), bool hasLine = false, bool page = true)
        : mRect(pt), mHasLine(hasLine), mSamePage(page)
    {
    }

    QRect mRect;
    bool mHasLine;
    bool mSamePage;
};

class PimPrint::Calendar::CalPrintTodos::Private
{
public:
    Private()
        : mTodoType(AllTodos),
          mHeaderText(i18nc("@title", "To-do List")),
          mSortField(KCalCore::TodoSortSummary),
          mSortDirection(KCalCore::SortDirectionAscending)
    {
    }

    QDate mStartDate;                       // starting date of print TODO(set a default?)
    QDate mEndDate;                         // ending date of print TODO (set a default?)
    TodoTypes mTodoType;                    // type of to-dos to print
    QString mHeaderText;                    // string for the header text
    KCalCore::TodoSortField mSortField;     // sort on this field
    KCalCore::SortDirection mSortDirection; // sort in this direction
};
//@endcond

CalPrintTodos::CalPrintTodos(QPrinter *printer)
    : CalPrintBase(printer), d(new PimPrint::Calendar::CalPrintTodos::Private)
{
    //TODO:
    // set the calendar and calendar system

    // Set default print style
    setPrintStyle(CalPrintBase::TodoList);

    // Set default Info options
    setInfoOptions(0);

    // Set default Type options
    setTypeOptions(0);

    // Set default Range options
    setRangeOptions(0);

    // Set default Extra options
    setExtraOptions(0);
}

CalPrintTodos::~CalPrintTodos()
{
    delete d;
}

void CalPrintTodos::setStartDate(const QDate &date)
{
    d->mStartDate = date;
}

QDate CalPrintTodos::startDate() const
{
    return d->mStartDate;
}

void CalPrintTodos::setEndDate(const QDate &date)
{
    d->mEndDate = date;
}

QDate CalPrintTodos::endDate() const
{
    return d->mEndDate;
}

void CalPrintTodos::setHeaderText(const QString &text)
{
    d->mHeaderText = text;
}

QString CalPrintTodos::headerText() const
{
    return d->mHeaderText;
}

void CalPrintTodos::setSortField(const KCalCore::TodoSortField &sortField)
{
    d->mSortField = sortField;
}

KCalCore::TodoSortField CalPrintTodos::sortField() const
{
    return d->mSortField;
}

void CalPrintTodos::setSortDirection(const KCalCore::SortDirection &sortDirection)
{
    d->mSortDirection = sortDirection;
}

KCalCore::SortDirection CalPrintTodos::sortDirection() const
{
    return d->mSortDirection;
}

void CalPrintTodos::print(QPainter &p)
{
    // TODO: Find a good way to guarantee a nicely designed output
    int pospriority = 0;
    int possummary = 100;
    int posdue = pageWidth() - 65;
    int poscomplete = posdue - 70; //Complete column is to right of the Due column
    int lineSpacing = 15;

    QRect headerBox(0, 0, pageWidth(), headerHeight());
    QRect footerBox(0, pageHeight() - footerHeight(), pageWidth(), footerHeight());
    int height = pageHeight() - footerHeight();

    // Draw the First Page Header
    drawHeader(p, headerBox, d->mHeaderText, d->mStartDate, QDate());

    // Draw the Column Headers
    int currentLinePos = headerHeight() + 5;
    QString outStr;
    QFont oldFont(p.font());

    p.setFont(QFont(QLatin1String("sans-serif"), 9, QFont::Bold));
    lineSpacing = p.fontMetrics().lineSpacing();
    currentLinePos += lineSpacing;
    if (infoOptions().testFlag(CalPrintBase::InfoPriority)) {
        outStr += i18nc("@title", "Priority");
        p.drawText(pospriority, currentLinePos - 2, outStr);
    } else {
        pospriority = -1;
    }

    outStr.truncate(0);
    outStr += i18nc("@label to-do summary", "Title");
    p.drawText(possummary, currentLinePos - 2, outStr);

    if (infoOptions().testFlag(CalPrintBase::InfoPercentDone)) {//TODO: should be true by default
        if (!infoOptions().testFlag(CalPrintBase::InfoDueDate)) {//TODO: should be true by default
            // Print Percent Complete in the Due Date column
            poscomplete = posdue;
        }
        outStr.truncate(0);
        outStr += i18nc("@label to-do percentage complete", "Complete");
        p.drawText(poscomplete, currentLinePos - 2, outStr);
    } else {
        poscomplete = -1;
    }

    if (infoOptions().testFlag(CalPrintBase::InfoDueDate)) {//TODO: should be true by default
        outStr.truncate(0);
        outStr += i18nc("@label to-do due date", "Due");
        p.drawText(posdue, currentLinePos - 2, outStr);
    } else {
        posdue = -1;
    }

    p.setFont(QFont(QLatin1String("sans-serif"), 10));

    KCalCore::Todo::List todoList;
    KCalCore::Todo::List tempList;

    // Create list of to-dos which will be printed
    todoList = printCalendar()->todos(d->mSortField, d->mSortDirection);
    switch (d->mTodoType) {
    case AllTodos:
        break;
    case Completed: //TODO: IMPLEMENT
        break;
    case NotStarted: // TODO: IMPLEMENT
        break;
    case OpenEnded: // TODO: IMPLEMENT
        break;
    case OverDue:  // TODO: IMPLEMENT
        break;
    case InProgressTodos:
        Q_FOREACH (const KCalCore::Todo::Ptr &todo, todoList) {
            Q_ASSERT(todo);
            if (!todo->isCompleted()) {
                tempList.append(todo);
            }
        }
        todoList = tempList;
        break;
    case DueDateRangeTodos:
        Q_FOREACH (const KCalCore::Todo::Ptr &todo, todoList) {
            Q_ASSERT(todo);
            if (todo->hasDueDate()) {
                if (todo->dtDue().date() >= d->mStartDate && todo->dtDue().date() <= d->mEndDate) {
                    tempList.append(todo);
                }
            } else {
                tempList.append(todo);
            }
        }
        todoList = tempList;
        break;
    }

    const bool printConf = typeOptions().testFlag(
                               CalPrintBase::TypeConfidential); //TODO should be false by default
    const bool printPrivate = typeOptions().testFlag(
                                  CalPrintBase::TypePrivate);   //TODO should be false by default

    // Print to-dos
    int count = 0;
    Q_FOREACH (const KCalCore::Todo::Ptr &todo, todoList) {
        if ((!printConf && todo->secrecy() == KCalCore::Incidence::SecrecyConfidential) ||
            (!printPrivate && todo->secrecy() == KCalCore::Incidence::SecrecyPrivate)) {
            continue;
        }
        // Skip sub-to-dos. They will be printed recursively in drawTodo()
        if (todo->relatedTo().isEmpty()) {   //review(AKONADI_PORT)
            count++;
            drawTodo(count, todo, p,
                     pospriority, possummary, posdue, poscomplete,
                     0, 0, currentLinePos, pageWidth(), height, todoList, 0);
        }
    }

    if (extraOptions().testFlag(CalPrintBase::ExtraFooter)) {
        drawFooter(p, footerBox);
    }
    p.setFont(oldFont);
}

void CalPrintTodos::drawTodo(int &count, const KCalCore::Todo::Ptr &todo, QPainter &p,
                             int posPriority, int posSummary,
                             int posDueDt, int posPercentComplete,
                             int level, int x, int &y,
                             int width, int pageHeight,
                             const KCalCore::Todo::List &todoList,
                             TodoParentStart *r)
{
    QString outStr;
    const KLocale *local = KLocale::global();  //TODO: set in ctor
    QRect rect;
    TodoParentStart startpt;
    // This list keeps all starting points of the parent to-dos so the connection
    // lines of the tree can easily be drawn (needed if a new page is started)
    static QList<TodoParentStart *> startPoints;
    if (level < 1) {
        startPoints.clear();
    }

    y += 10;

    // Compute the right hand side of the to-do box
    int rhs = posPercentComplete;
    if (rhs < 0) {
        rhs = posDueDt; //not printing percent completed
    }
    if (rhs < 0) {
        rhs = x + width;  //not printing due dates either
    }

    int left = posSummary + (level * 10);

    // If this is a sub-to-do, r will not be 0, and we want the LH side
    // of the priority line up to the RH side of the parent to-do's priority
    bool showPriority = posPriority >= 0;
    int lhs = posPriority;
    if (r) {
        lhs = r->mRect.right() + 1;
    }

    outStr.setNum(todo->priority());
    rect = p.boundingRect(lhs, y + 10, 5, -1, Qt::AlignCenter, outStr);
    // Make it a more reasonable size
    rect.setWidth(18);
    rect.setHeight(18);

    // Draw a checkbox
    p.setBrush(QBrush(Qt::NoBrush));
    p.drawRect(rect);
    if (todo->isCompleted()) {
        // cross out the rectangle for completed to-dos
        p.drawLine(rect.topLeft(), rect.bottomRight());
        p.drawLine(rect.topRight(), rect.bottomLeft());
    }
    lhs = rect.right() + 3;

    // Priority
    if (todo->priority() > 0 && showPriority) {
        p.drawText(rect, Qt::AlignCenter, outStr);
    }
    startpt.mRect = rect; //save for later

    // Connect the dots
    if (extraOptions().testFlag(CalPrintBase::ExtraConnectSubTodos)) {//TODO should be true by default
        if (r && level > 0) {
            int bottom;
            int center(r->mRect.left() + (r->mRect.width() / 2));
            int to(rect.top() + (rect.height() / 2));
            int endx(rect.left());
            p.drawLine(center, to, endx, to);    // side connector
            if (r->mSamePage) {
                bottom = r->mRect.bottom() + 1;
            } else {
                bottom = 0;
            }
            p.drawLine(center, bottom, center, to);
        }
    }

    // summary
    outStr = todo->summary();
    rect = p.boundingRect(lhs, rect.top(), (rhs - (left + rect.width() + 5)),
                          -1, Qt::TextWordWrap, outStr);

    QRect newrect;
    QFont newFont(p.font());
    QFont oldFont(p.font());
    if (extraOptions().testFlag(CalPrintBase::ExtraStrikeDoneTodos) &&
        todo->isCompleted()) {//TODO: should be false by default
        newFont.setStrikeOut(true);
        p.setFont(newFont);
    }
    p.drawText(rect, Qt::TextWordWrap, outStr, &newrect);
    p.setFont(oldFont);
    // due date
    if (todo->hasDueDate() && posDueDt >= 0) {
        outStr = local->formatDate(todo->dtDue().toLocalZone().date(), KLocale::ShortDate);
        rect = p.boundingRect(posDueDt, y, x + width, -1,
                              Qt::AlignTop | Qt::AlignLeft, outStr);
        p.drawText(rect, Qt::AlignTop | Qt::AlignLeft, outStr);
    }

    // percentage completed
    bool showPercentComplete = posPercentComplete >= 0;
    if (showPercentComplete) {
        int lwidth = 24;
        int lheight = 12;
        //first, draw the progress bar
        int progress = (int)((lwidth * todo->percentComplete()) / 100.0 + 0.5);

        p.setBrush(QBrush(Qt::NoBrush));
        p.drawRect(posPercentComplete, y + 3, lwidth, lheight);
        if (progress > 0) {
            p.setBrush(QColor(128, 128, 128));
            p.drawRect(posPercentComplete, y + 3, progress, lheight);
        }

        //now, write the percentage
        outStr = i18nc("@item the percent completed of a to-do", "%1%", todo->percentComplete());
        rect = p.boundingRect(posPercentComplete + lwidth + 3, y, x + width, -1,
                              Qt::AlignTop | Qt::AlignLeft, outStr);
        p.drawText(rect, Qt::AlignTop | Qt::AlignLeft, outStr);
    }

    const bool printConf = typeOptions().testFlag(
                               CalPrintBase::TypeConfidential); //TODO should be false by default
    const bool printPrivate = typeOptions().testFlag(
                                  CalPrintBase::TypePrivate);   //TODO should be false by default

    y += 10;

    // Make a list of all the sub-to-dos related to this to-do.
    KCalCore::Todo::List t;
    KCalCore::Incidence::List relations = printCalendar()->relations(todo->uid());

    Q_FOREACH (const KCalCore::Incidence::Ptr &incidence, relations) {
        // In the future, to-dos might also be related to events
        // Manually check if the sub-to-do is in the list of to-dos to print
        // The problem is that relations() does not apply filters, so
        // we need to compare manually with the complete filtered list!
        KCalCore::Todo::Ptr subtodo = incidence.dynamicCast<KCalCore::Todo>();
        if (!subtodo) {
            continue;
        }
#ifdef AKONADI_PORT_DISABLED
        if (subtodo && todoList.contains(subtodo)) {
#else
        bool subtodoOk = false;
        if (subtodo) {
            Q_FOREACH (const KCalCore::Todo::Ptr &tt, todoList) {
                if (tt == subtodo) {
                    subtodoOk = true;
                    break;
                }
            }
        }
        if (subtodoOk) {
#endif
            if ((!printConf && subtodo->secrecy() == KCalCore::Incidence::SecrecyConfidential) ||
                (!printPrivate && subtodo->secrecy() == KCalCore::Incidence::SecrecyPrivate)) {
                continue;
            }
            t.append(subtodo);
        }
    }

    // has sub-todos?
    startpt.mHasLine = (relations.size() > 0);
    startPoints.append(&startpt);

    // description
    if (infoOptions().testFlag(CalPrintBase::InfoDescription) && //TODO: should be false by default
        !todo->description().isEmpty()) {
        y = newrect.bottom() + 5;
        drawTodoLines(p, todo->description(), left, y,
                      width - (left + 10 - x), pageHeight,
                      todo->descriptionIsRich(),
                      startPoints);
    } else {
        y += 10;
    }

    // Sort the sub-to-dos and print them
#ifdef AKONADI_PORT_DISABLED
    KCalCore::Todo::List sl = printCalendar()->sortTodos(&t, d->mSortField, d->mSortDirection);
#else
    KCalCore::Todo::List tl;
    Q_FOREACH (const KCalCore::Todo::Ptr &todo, t) {
        tl.append(todo);
    }
    KCalCore::Todo::List sl = printCalendar()->sortTodos(tl, d->mSortField, d->mSortDirection);
#endif

    int subcount = 0;
    Q_FOREACH (const KCalCore::Todo::Ptr &isl, sl) {
        count++;
        if (++subcount == sl.size()) {
            startpt.mHasLine = false;
        }
        drawTodo(count, isl, p,
                 posPriority, posSummary, posDueDt, posPercentComplete,
                 level + 1, x, y, width, pageHeight, todoList, &startpt);
    }
    startPoints.removeAll(&startpt);
}

void CalPrintTodos::drawTodoLines(QPainter &p,
                                  const QString &description,
                                  int x, int &y,
                                  int width, int pageHeight,
                                  bool richTextDescription,
                                  QList<TodoParentStart *> &startPoints)
{
    QString plainDesc = (richTextDescription) ? toPlainText(description) : description;

    QRect textrect(0, 0, width, -1);
    int flags = Qt::AlignLeft;
    QFontMetrics fm = p.fontMetrics();  //TODO: set in ctor

    QStringList lines = plainDesc.split(QLatin1Char('\n'));
    for (int currentLine = 0; currentLine < lines.count(); currentLine++) {
        // split paragraphs into lines
        KWordWrap *ww = KWordWrap::formatText(fm, textrect, flags, lines[currentLine]);
        QStringList textLine = ww->wrappedString().split(QLatin1Char('\n'));
        delete ww;

        // print each individual line
        for (int lineCount = 0; lineCount < textLine.count(); lineCount++) {
            if (y >= pageHeight) {
                if (extraOptions().testFlag(CalPrintBase::ExtraConnectSubTodos)) {//TODO should be true by default
                    for (int i = 0; i < startPoints.size(); ++i) {
                        TodoParentStart *rct;
                        rct = startPoints.at(i);
                        int start = rct->mRect.bottom() + 1;
                        int center = rct->mRect.left() + (rct->mRect.width() / 2);
                        int to = y;
                        if (!rct->mSamePage) {
                            start = 0;
                        }
                        if (rct->mHasLine) {
                            p.drawLine(center, start, center, to);
                        }
                        rct->mSamePage = false;
                    }
                }
                y = 0;
                thePrinter()->newPage();
            }
            y += fm.height();
            p.drawText(x, y, textLine[lineCount]);
        }
    }
}

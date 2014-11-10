/*
  This file is part of KOrganizer.

  Copyright (c) 2004 Till Adam <adam@kde.org>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>
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
  51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "todoviewquicksearch.h"
#include <calendarsupport/kcalprefs.h>

#include <Akonadi/Calendar/ETMCalendar>
#include <calendarsupport/categoryconfig.h>

#include <libkdepim/widgets/kcheckcombobox.h>
#include <libkdepim/widgets/tagwidgets.h>

#include <calendarsupport/categoryhierarchyreader.h>

#include <KCalCore/CalFilter>

#include <QLineEdit>
#include <KLocalizedString>

#include <QHBoxLayout>

TodoViewQuickSearch::TodoViewQuickSearch(const Akonadi::ETMCalendar::Ptr &calendar,
        QWidget *parent)
    : QWidget(parent), mCalendar(calendar)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    // no special margin because it is added by the view
    layout->setContentsMargins(0, 0, 0, 0);

    mSearchLine = new QLineEdit(this);
    mSearchLine->setToolTip(
        i18nc("@info:tooltip", "Filter on matching summaries"));
    mSearchLine->setWhatsThis(
        i18nc("@info:whatsthis",
              "Enter text here to filter the to-dos that are shown by matching summaries."));
    mSearchLine->setPlaceholderText(i18nc("@label in QuickSearchLine", "Search Summaries"));
    mSearchLine->setClearButtonEnabled(true);
    connect(mSearchLine, SIGNAL(textChanged(QString)),
            this, SIGNAL(searchTextChanged(QString)));

    layout->addWidget(mSearchLine, 3);

    mCategoryCombo = new KPIM::TagSelectionCombo(this);
    mCategoryCombo->setToolTip(
        i18nc("@info:tooltip", "Filter on these categories"));
    mCategoryCombo->setWhatsThis(
        i18nc("@info:whatsthis",
              "Use this combobox to filter the to-dos that are shown by "
              "a list of selected categories."));
    const QString defaultText = i18nc("@item:inlistbox", "Select Categories");
    mCategoryCombo->setDefaultText(defaultText);
    mCategoryCombo->setSeparator(i18nc("@item:intext delimiter for joining category names", ","));

    connect(mCategoryCombo, SIGNAL(checkedItemsChanged(QStringList)),
            SLOT(emitFilterCategoryChanged()));

    layout->addWidget(mCategoryCombo, 1);

    {
        // Make the combo big enough so that "Select Categories" fits.
        QFontMetrics fm = mCategoryCombo->lineEdit()->fontMetrics();

        // QLineEdit::sizeHint() returns a nice size to fit 17 'x' chars.
        const int currentPreferedWidth = mCategoryCombo->lineEdit()->sizeHint().width();

        // Calculate a nice size for "Select Categories"
        const int newPreferedWidth = currentPreferedWidth -
                                     fm.width(QLatin1Char('x')) * 17 +
                                     fm.width(defaultText);

        const int pixelsToAdd = newPreferedWidth - mCategoryCombo->lineEdit()->width();
        mCategoryCombo->setMinimumWidth(mCategoryCombo->width() + pixelsToAdd);
    }

    mPriorityCombo = new KPIM::KCheckComboBox(this);
    mPriorityCombo->setToolTip(
        i18nc("@info:tooltip", "Filter on these priorities"));
    mPriorityCombo->setWhatsThis(
        i18nc("@info:whatsthis",
              "Use this combobox to filter the to-dos that are shown by "
              "a list of selected priorities."));
    mPriorityCombo->setDefaultText(i18nc("@item:inlistbox", "Select Priority"));
    connect(mPriorityCombo, SIGNAL(checkedItemsChanged(QStringList)),
            SLOT(emitFilterPriorityChanged()));

    layout->addWidget(mPriorityCombo, 1);
    fillPriorities();

    setLayout(layout);
}

void TodoViewQuickSearch::setCalendar(const Akonadi::ETMCalendar::Ptr &calendar)
{
    if (calendar != mCalendar) {
        mCalendar = calendar;
    }
}

void TodoViewQuickSearch::reset()
{
    mSearchLine->clear();
    mCategoryCombo->setCurrentIndex(0);
    mPriorityCombo->setCurrentIndex(0);
}

void TodoViewQuickSearch::fillPriorities()
{
    QStringList priorityValues;
    priorityValues.append(i18nc("@action:inmenu priority is unspecified", "unspecified"));
    priorityValues.append(i18nc("@action:inmenu highest priority", "%1 (highest)", 1));
    for (int p = 2; p < 10; ++p) {
        if (p == 5) {
            priorityValues.append(i18nc("@action:inmenu medium priority", "%1 (medium)", p));
        } else if (p == 9) {
            priorityValues.append(i18nc("@action:inmenu lowest priority", "%1 (lowest)", p));
        } else {
            priorityValues.append(i18nc("@action:inmenu", "%1", p));
        }
    }
    // TODO: Using the same method as for categories to fill mPriorityCombo
    CalendarSupport::CategoryHierarchyReaderQComboBox(mPriorityCombo).read(priorityValues);
}

void TodoViewQuickSearch::emitFilterCategoryChanged()
{
    emit filterCategoryChanged(mCategoryCombo->checkedItems());
}

void TodoViewQuickSearch::emitFilterPriorityChanged()
{
    emit filterPriorityChanged(mPriorityCombo->checkedItems(Qt::UserRole));
}


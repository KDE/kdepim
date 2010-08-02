/*
 *   This file only:
 *     Copyright (C) 2003  Mark Bucciarelli <mark@hubcapconsutling.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the
 *      Free Software Foundation, Inc.
 *      51 Franklin Street, Fifth Floor
 *      Boston, MA  02110-1301  USA.
 *
 */
#ifndef KARM_PRINT_DIALOG_H
#define KARM_PRINT_DIALOG_H

#include <kdialogbase.h>
#include <libkdepim/kdateedit.h>

class TQCheckBox;
class KDateEdit;

class PrintDialog : public KDialogBase
{
  Q_OBJECT

  public:
    PrintDialog();

    /* Return the from date entered.  */
    TQDate from() const;

    /* Return the to date entered.  */
    TQDate to() const;

    /* Whether to summarize per week */
    bool perWeek() const;

    /* Whether to print all tasks */
    bool allTasks() const;

    /* Whether to print totals only, instead of per-day columns */
    bool totalsOnly() const;

private:
    KDateEdit *_from, *_to;
    TQCheckBox *_perWeek;
    TQComboBox *_allTasks;
    TQCheckBox *_totalsOnly;
};

#endif // KARM_PRINT_DIALOG_H


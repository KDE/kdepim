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

#include <tqbuttongroup.h>
#include <tqcheckbox.h>
#include <tqhbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqlineedit.h>
#include <tqpixmap.h>
#include <tqpushbutton.h>
#include <tqstring.h>
#include <tqwidget.h>
#include <tqwhatsthis.h>

#include <kiconloader.h>
#include <klocale.h>            // i18n
#include <kwinmodule.h>

#include "printdialog.h"
#include <libkdepim/kdateedit.h>


PrintDialog::PrintDialog()
  : KDialogBase(0, "PrintDialog", true, i18n("Print Dialog"), Ok|Cancel,
      Ok, true )
{
  TQWidget *page = new TQWidget( this );
  setMainWidget(page);
  int year, month;

  TQVBoxLayout *layout = new TQVBoxLayout(page, KDialog::spacingHint());
  layout->addSpacing(10);
  layout->addStretch(1);

  // Date Range
  TQGroupBox *rangeGroup = new TQGroupBox(1, Horizontal, i18n("Date Range"),
      page);
  layout->addWidget(rangeGroup);

  TQWidget *rangeWidget = new TQWidget(rangeGroup);
  TQHBoxLayout *rangeLayout = new TQHBoxLayout(rangeWidget, 0, spacingHint());

  rangeLayout->addWidget(new TQLabel(i18n("From:"), rangeWidget));
  _from = new KDateEdit(rangeWidget);

  // Default from date to beginning of the month
  year = TQDate::currentDate().year();
  month = TQDate::currentDate().month();
  _from->setDate(TQDate(year, month, 1));
  rangeLayout->addWidget(_from);
  rangeLayout->addWidget(new TQLabel(i18n("To:"), rangeWidget));
  _to = new KDateEdit(rangeWidget);
  rangeLayout->addWidget(_to);

  layout->addSpacing(10);
  layout->addStretch(1);

  _allTasks = new TQComboBox( page );
  _allTasks->insertItem( i18n( "Selected Task" ) );
  _allTasks->insertItem( i18n( "All Tasks" ) );
  layout->addWidget( _allTasks );

  _perWeek = new TQCheckBox( i18n( "Summarize per week" ), page );
  layout->addWidget( _perWeek );
  _totalsOnly = new TQCheckBox( i18n( "Totals only" ), page );
  layout->addWidget( _totalsOnly );

  layout->addSpacing(10);
  layout->addStretch(1);
}

TQDate PrintDialog::from() const
{
  return _from->date();
}

TQDate PrintDialog::to() const
{
  return _to->date();
}

bool PrintDialog::perWeek() const
{
  return _perWeek->isChecked();
}

bool PrintDialog::allTasks() const
{
  return _allTasks->currentItem() == 1;
}

bool PrintDialog::totalsOnly() const
{
  return _totalsOnly->isChecked();
}

#include "printdialog.moc"

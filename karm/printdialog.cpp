/*
 *   This program is free software; you can redistribute it and/or modify it
 *   under the terms of the GNU General Public License as published by the
 *   Free Software Foundation; either version 2 of the License, or (at your
 *   option) any later version.
 *
 *   This program is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 *   Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qwidget.h>
#include <qwhatsthis.h>

#include <kiconloader.h>
#include <klocale.h>            // i18n
#include <kwinmodule.h>

#include "printdialog.h"
#include <libkdepim/kdateedit.h>


PrintDialog::PrintDialog()
  : KDialogBase(0, "PrintDialog", true, i18n("Print Dialog"), Ok|Cancel,
      Ok, true )
{
  QWidget *page = new QWidget( this ); 
  setMainWidget(page);

  QVBoxLayout *layout = new QVBoxLayout(page);
  
  layout->addSpacing(10);
  layout->addStretch(1);
  
  // Date Range
  QGroupBox *rangeGroup = new QGroupBox(1, Horizontal, i18n("Date Range"), 
      page);
  layout->addWidget(rangeGroup);

  QWidget *rangeWidget = new QWidget(rangeGroup);
  QHBoxLayout *rangeLayout = new QHBoxLayout(rangeWidget, 0, spacingHint());

  rangeLayout->addWidget(new QLabel(i18n("From:"), rangeWidget));
  _from = new KDateEdit(rangeWidget);
  _from->setDate(QDate::currentDate().addDays(-1));
  rangeLayout->addWidget(_from);
  rangeLayout->addWidget(new QLabel(i18n("To:"), rangeWidget));
  _to = new KDateEdit(rangeWidget);
  rangeLayout->addWidget(_to);

  layout->addSpacing(10);
  layout->addStretch(1);

  layout->addWidget(new QLabel(QString::fromLatin1("STUB: "
          "This report doesn't work yet."), page));

  layout->addSpacing(10);
  layout->addStretch(1);
}

QDate PrintDialog::from() const
{
  return _from->date();
}

QDate PrintDialog::to() const
{
  return _to->date();
}

#include "printdialog.moc"

/*
 *   Copyright (C) 2004  Mark Bucciarelli <mark@hubcapconsulting.com>
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
 *      59 Temple Place - Suite 330
 *      Boston, MA  02111-1307  USA.
 *
 */
#include <kdateedit.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klineedit.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kurlrequester.h>
#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qradiobutton.h>

#include "csvexportdialog.h"
#include "reportcriteria.h"

CSVExportDialog::CSVExportDialog( ReportCriteria::REPORTTYPE rt,
                                  QWidget *parent, 
                                  const char *name
                                  ) 
  : CSVExportDialogBase( parent, name )
{
  switch ( rt ) {
    case ReportCriteria::CSVTotalsExport:
      grpDateRange->setEnabled( false );
      grpDateRange->hide();
      rc.reportType = rt;
      break;
    case ReportCriteria::CSVHistoryExport:
      grpDateRange->setEnabled( true );
      rc.reportType = rt;
      break;
    default:
      break;

  }

  // If decimal symbol is a comma, then default field seperator to semi-colon.
  // In France and Germany, one-and-a-half is written as 1,5 not 1.5
  QString d = KGlobal::locale()->decimalSymbol();
  if ( "," == d ) CSVExportDialogBase::radioSemicolon->setChecked(true);
  else CSVExportDialogBase::radioComma->setChecked(true);

}

void CSVExportDialog::enableExportButton()
{
  btnExport->setEnabled( !urlExportTo->lineEdit()->text().isEmpty() );
}

void CSVExportDialog::enableTasksToExportQuestion()
{
  return;
  //grpTasksToExport->setEnabled( true );      
}

ReportCriteria CSVExportDialog::reportCriteria()
{
  rc.url = urlExportTo->url();
  rc.from = dtFrom->date();
  rc.to = dtTo->date();

  // Hard code to true for now as the CSV export of totals does not support
  // this choice currenly and I'm trying to minimize pre-3.3 hacking at the
  // moment.
  rc.allTasks = true;

  QString t = grpTimeFormat->selected()->text(); 
  rc.decimalMinutes = ( t == i18n( "Decimal" ) );

  QString d = grpDelimiter->selected()->text(); 
  if      ( d == i18n( "Comma" ) )     rc.delimiter = ",";
  else if ( d == i18n( "Tab" ) )       rc.delimiter = "\t";
  else if ( d == i18n( "Semicolon" ) ) rc.delimiter = ";";
  else if ( d == i18n( "Space" ) )     rc.delimiter = " ";
  else if ( d == i18n( "Other:" ) )     rc.delimiter = txtOther->text();
  else {
    kdDebug(5970) 
      << "*** CSVExportDialog::reportCriteria: Unexpected delimiter choice '" 
      << d << "'--defaulting to a tab" << endl;
    rc.delimiter = "\t";
  }

  rc.quote = cboQuote->currentText();

  return rc;
}

#include "csvexportdialog.moc"

#include <kdebug.h>
#include <kdateedit.h>
#include <klineedit.h>
#include <klocale.h>            // i18n
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
      rc.reportType = rt;
      break;
    case ReportCriteria::CSVHistoryExport:
      grpDateRange->setEnabled( false );      
      rc.reportType = rt;
      break;
    default:
      break;
  }
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

#ifndef REPORTCRITERIA_H
#define REPORTCRITERIA_H

#include <qdatetime.h>
class QString;

/**
 Stores entries from export dialog.

 Keeps details (like CSV export dialog control names) out of the TaskView
 class, which contains the slot triggered by the export action.  
 
 The dialog and the report logic can change all they want and the TaskView
 logic can stay the same.
 */

class ReportCriteria
{
  public:

    /**
    The different report types.
    */
    enum REPORTTYPE { CSVTotalsExport = 0, CSVHistoryExport = 1 };

    /**
    The type of report we are running.
    */
    REPORTTYPE reportType;

    /**
     For reports that write to a file, the filename to write to.
     */
    QString url;

    /**
     For history reports, the lower bound of the date range to report on.
     */
    QDate   from;

    /**
     For history reports, the upper bound of the date range to report on.
     */
    QDate   to;

    /**
     True if the report should contain all tasks in Karm.

     Defaults to true.
     */
    bool    allTasks;

    /**
     True if the durations should be output in decimal hours.  Otherwise,
     output durations as HH24:MI
     */
    bool    decimalMinutes;

    /**
     The delimiter to use when outputting comma-seperated value reports.
     */
    QString delimiter;

    /**
     The quote to use for text fields when outputting comma-seperated reports.
     */
    QString quote;
};

#endif

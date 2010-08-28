/*=========================================================================
| KCalDAV
|--------------------------------------------------------------------------
| (c) 2010  Timothy Pearson
| (c) 2009  Kumaran Santhanam (initial KDE4 version)
|
| This project is released under the GNU General Public License.
| Please see the file COPYING for more details.
|--------------------------------------------------------------------------
| Remote calendar loading.
 ========================================================================*/

/*=========================================================================
| INCLUDES
 ========================================================================*/

#include "reader.h"
#include <kdebug.h>
#include <string>

/*=========================================================================
| NAMESPACE
 ========================================================================*/

using namespace KCal;

/*=========================================================================
| METHODS
 ========================================================================*/

void CalDavReader::cleanJob() {
    CalDavJob::cleanJob();
    mData = "";
    mTasksData = "";
}

int CalDavReader::runJob(runtime_info* RT) {
    kdDebug() << "reader::run, url: " << url();

    response* result = caldav_get_response();
    CALDAV_RESPONSE res = OK;

    if (mGetAll) {
        kdDebug() << "getting all objects";
        res = caldav_tasks_getall_object(result, std::string(url().ascii()).c_str(), RT);
    } else {
        kdDebug() << "getting object from the specified time range";
        res = caldav_tasks_get_object(result, mTimeStart.toTime_t(), mTimeEnd.toTime_t(), std::string(url().ascii()).c_str(), RT);
    }

    if (OK == res) {
        kdDebug() << "success";
        if (result->msg) {
            mData = result->msg;
        } else {
            kdDebug() << "empty collection";
            // empty collection
            mData = "";
        }
    }

    caldav_free_response(&result);

    if ((OK == res) && (tasksUrl() != "")) {
      kdDebug() << "reader::run, url: " << tasksUrl();

      response* result = caldav_get_response();
      CALDAV_RESPONSE res = OK;

      if (mGetAll) {
          kdDebug() << "getting all objects";
          res = caldav_tasks_getall_object(result, std::string(tasksUrl().ascii()).c_str(), RT);
      } else {
          kdDebug() << "getting object from the specified time range";
          res = caldav_tasks_get_object(result, mTimeStart.toTime_t(), mTimeEnd.toTime_t(), std::string(tasksUrl().ascii()).c_str(), RT);
      }

      if (OK == res) {
          kdDebug() << "success";
          if (result->msg) {
              mTasksData = result->msg;
          } else {
              kdDebug() << "empty collection";
              // empty collection
              mTasksData = "";
          }
      }

      caldav_free_response(&result);
    }

    return res;
}

// EOF ========================================================================

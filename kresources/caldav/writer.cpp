/*=========================================================================
| KCalDAV
|--------------------------------------------------------------------------
| (c) 2010  Timothy Pearson
| (c) 2009  Kumaran Santhanam (initial KDE4 version)
|
| This project is released under the GNU General Public License.
| Please see the file COPYING for more details.
|--------------------------------------------------------------------------
| Remote calendar writing class.
 ========================================================================*/

/*=========================================================================
| INCLUDES
 ========================================================================*/

#include "writer.h"
#include <kdebug.h>
#include <string>

/*=========================================================================
| DEFINES
 ========================================================================*/

// Use caldav_modify_object() function.
// If it's not set, a pair of caldav_delete_object/caldav_add_object
// is used for modifying objects.
// It's done, because, for some reason, SOGo server returns an error
// on caldav_modify_object. DAViCAL works fine both ways.
#define USE_CALDAV_MODIFY
#define USE_CALDAV_TASKS_MODIFY

/*=========================================================================
| NAMESPACE
 ========================================================================*/

using namespace KCal;

/*=========================================================================
| METHODS
 ========================================================================*/

void CalDavWriter::cleanJob() {
    CalDavJob::cleanJob();
}

int CalDavWriter::runJob(runtime_info* RT) {
    kdDebug() << "writer::run, url: " << url() << "\n";

    int res = OK;

    if ((OK == res) && (url() != "")) {
      kdDebug() << "pushing added objects";
      res = pushObjects(mAdded, caldav_add_object, OK, RT);
      if (OK == res) {
#ifdef USE_CALDAV_MODIFY
          kdDebug() << "pushing changed objects";
          res = pushObjects(mChanged, caldav_modify_object, OK, RT);
          if (OK == res) {
              kdDebug() << "pushing deleted objects";
              res = pushObjects(mDeleted, caldav_delete_object, OK, RT);
          }
#else // if USE_CALDAV_MODIFY
          kdDebug() << "pushing changed objects (delete)";
          res = pushObjects(mChanged, caldav_delete_object, OK, RT);
          if (OK == res) {
              kdDebug() << "pushing changed objects (add)";
              res = pushObjects(mChanged, caldav_add_object, OK, RT);
              if (OK == res) {
                  kdDebug() << "pushing deleted objects";
                  res = pushObjects(mDeleted, caldav_delete_object, OK, RT);
              }
          }
#endif // if USE_CALDAV_MODIFY
      }
    }

    int tasksres = OK;

    if ((OK == tasksres) && (tasksUrl() != "")) {
      kdDebug() << "pushing added tasks objects";
      tasksres = pushTasksObjects(mTasksAdded, caldav_add_object, OK, RT);
      if (OK == tasksres) {
#ifdef USE_CALDAV_TASKS_MODIFY
          kdDebug() << "pushing changed objects";
          tasksres = pushTasksObjects(mTasksChanged, caldav_tasks_modify_object, OK, RT);
          if (OK == tasksres) {
              kdDebug() << "pushing deleted objects";
              tasksres = pushTasksObjects(mTasksDeleted, caldav_tasks_delete_object, OK, RT);
          }
#else // if USE_CALDAV_TASKS_MODIFY
          kdDebug() << "pushing changed objects (delete)";
          tasksres = pushTasksObjects(mTasksChanged, caldav_tasks_delete_object, OK, RT);
          if (OK == tasksres) {
              kdDebug() << "pushing changed objects (add)";
              tasksres = pushTasksObjects(mTasksChanged, caldav_add_object, OK, RT);
              if (OK == tasksres) {
                  kdDebug() << "pushing deleted objects";
                  tasksres = pushTasksObjects(mTasksDeleted, caldav_tasks_delete_object, OK, RT);
              }
          }
#endif // if USE_CALDAV_TASKS_MODIFY
      }
    }

    int journalsres = OK;

    if ((OK == journalsres) && (tasksUrl() != "")) {
      kdDebug() << "pushing added tasks objects";
      journalsres = pushJournalsObjects(mJournalsAdded, caldav_add_object, OK, RT);
      if (OK == journalsres) {
#ifdef USE_CALDAV_TASKS_MODIFY
          kdDebug() << "pushing changed objects";
          journalsres = pushJournalsObjects(mJournalsChanged, caldav_tasks_modify_object, OK, RT);
          if (OK == journalsres) {
              kdDebug() << "pushing deleted objects";
              journalsres = pushJournalsObjects(mJournalsDeleted, caldav_tasks_delete_object, OK, RT);
          }
#else // if USE_CALDAV_TASKS_MODIFY
          kdDebug() << "pushing changed objects (delete)";
          journalsres = pushJournalsObjects(mJournalsChanged, caldav_tasks_delete_object, OK, RT);
          if (OK == journalsres) {
              kdDebug() << "pushing changed objects (add)";
              journalsres = pushJournalsObjects(mJournalsChanged, caldav_add_object, OK, RT);
              if (OK == journalsres) {
                  kdDebug() << "pushing deleted objects";
                  journalsres = pushJournalsObjects(mJournalsDeleted, caldav_tasks_delete_object, OK, RT);
              }
          }
#endif // if USE_CALDAV_TASKS_MODIFY
      }
    }

    if ((OK != res) || (OK != tasksres)) {
        clearObjects();
    }

    if (tasksres == OK)
      return res;
    else
      return tasksres;
}

int CalDavWriter::runTasksJob(runtime_info* RT) {
    // Stub function as there is no reason to split the writing jobs like the reading jobs
    return OK;
}

void CalDavWriter::cleanTasksJob() {
    // Stub function as there is no reason to split the writing jobs like the reading jobs
}

int CalDavWriter::runJournalsJob(runtime_info* RT) {
    // Stub function as there is no reason to split the writing jobs like the reading jobs
    return OK;
}

void CalDavWriter::cleanJournalsJob() {
    // Stub function as there is no reason to split the writing jobs like the reading jobs
}

// EOF ========================================================================

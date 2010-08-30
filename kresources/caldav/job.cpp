/*=========================================================================
| KCalDAV
|--------------------------------------------------------------------------
| (c) 2010  Timothy Pearson
| (c) 2009  Kumaran Santhanam (initial KDE4 version)
|
| This project is released under the GNU General Public License.
| Please see the file COPYING for more details.
|--------------------------------------------------------------------------
| Job class for accessing remote calendars.
 ========================================================================*/

/*=========================================================================
| INCLUDES
 ========================================================================*/

#include "job.h"
#include <kdebug.h>
#include <klocale.h>

#include <tqmutex.h>

#define log(s)      kdDebug() << s;

/*=========================================================================
| NAMESPACE
 ========================================================================*/

using namespace KCal;

/*=========================================================================
| STATIC
 ========================================================================*/

/*=========================================================================
| CONSTRUCTOR AND DESTRUCTOR
 ========================================================================*/

CalDavJob::CalDavJob(const TQString& url) {
    cleanJob();
    setUrl(url);
}

CalDavJob::~CalDavJob() {
}


/*=========================================================================
| METHODS
 ========================================================================*/

void CalDavJob::enableCaldavDebug(runtime_info* rt) {
    if (rt && rt->options) {
        rt->options->debug = 0; // if debug = 1, it causes major CPU overhead
        rt->options->verify_ssl_certificate = FALSE;
    }
}

void CalDavJob::setErrorString(const TQString& err, const long number) {
    mError = true;
    mErrorString = err;
    mErrorNumber = number;
}

void CalDavJob::setTasksErrorString(const TQString& err, const long number) {
    mTasksError = true;
    mTasksErrorString = err;
    mTasksErrorNumber = number;
}

void CalDavJob::processError(const caldav_error* err) {
    TQString error_string;

    long code = err->code;

    if (-401 == code) { // unauthorized
        error_string = i18n("Unauthorized. Username or password incorrect.");
    } else if (-599 <= code && code <= -300) {
        error_string = i18n("HTTP error %1. Please ensure that the URL is a valid CalDAV resource.").arg(-code);
    } else {
        error_string = err->str;
    }

    setErrorString(error_string, code);
}

void CalDavJob::processTasksError(const caldav_error* err) {
    TQString error_string;

    long code = err->code;

    if (-401 == code) { // unauthorized
        error_string = i18n("Unauthorized. Username or password incorrect.");
    } else if (-599 <= code && code <= -300) {
        error_string = i18n("HTTP error %1. Please ensure that the URL is a valid CalDAV resource.").arg(-code);
    } else {
        error_string = err->str;
    }

    setTasksErrorString(error_string, code);
}


void CalDavJob::run() {
    log("cleaning job");
    cleanJob();

    int res = OK;
    int tasksres = OK;

    runtime_info* caldav_runtime = caldav_get_runtime_info();

#ifdef KCALDAV_DEBUG
    log("setting debug caldav options");
    enableCaldavDebug(caldav_runtime);
#endif // KCALDAV_DEBUG

    log("running event job");
    res = runJob(caldav_runtime);

    if (OK != res) {
        log("event job failed");
        processError(caldav_runtime->error);
    }

    log("running tasks job");
    tasksres = runTasksJob(caldav_runtime);

    if (OK != tasksres) {
        log("tasks job failed");
        processTasksError(caldav_runtime->error);
    }

    caldav_free_runtime_info(&caldav_runtime);

    // Signal done
    // 1000 is read, 1001 is write
    if (type() == 0) TQApplication::postEvent ( parent(), new TQEvent( static_cast<TQEvent::Type>(1000) ) );
    if (type() == 1) TQApplication::postEvent ( parent(), new TQEvent( static_cast<TQEvent::Type>(1001) ) );
}

// EOF ========================================================================

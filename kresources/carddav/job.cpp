/*=========================================================================
| KCardDAV
|--------------------------------------------------------------------------
| (c) 2010  Timothy Pearson
|
| This project is released under the GNU General Public License.
| Please see the file COPYING for more details.
|--------------------------------------------------------------------------
| Job class for accessing remote address books.
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

using namespace KABC;

/*=========================================================================
| STATIC
 ========================================================================*/

/*=========================================================================
| CONSTRUCTOR AND DESTRUCTOR
 ========================================================================*/

CardDavJob::CardDavJob(const TQString& url) : mUseURI(false) {
    cleanJob();
    setUrl(url);
}

CardDavJob::~CardDavJob() {
}


/*=========================================================================
| METHODS
 ========================================================================*/

void CardDavJob::enableCarddavDebug(runtime_info* rt) {
    if (rt && rt->options) {
        rt->options->debug = 0; // if debug = 1, it causes major CPU overhead
        rt->options->verify_ssl_certificate = FALSE;
    }
}

void CardDavJob::setErrorString(const TQString& err, const long number) {
    mError = true;
    mErrorString = err;
    mErrorNumber = number;
}

void CardDavJob::processError(const carddav_error* err) {
    TQString error_string;

    long code = err->code;

    if (-401 == code) { // unauthorized
        error_string = i18n("Unauthorized. Username or password incorrect.");
    } else if (-599 <= code && code <= -300) {
        error_string = i18n("HTTP error %1. Maybe, URL is not a CardDAV resource.").arg(-code);
    } else {
        error_string = err->str;
    }

    setErrorString(error_string, code);
}


void CardDavJob::run() {
    log("cleaning job");
    cleanJob();

    int res = OK;

    runtime_info* carddav_runtime = carddav_get_runtime_info();

#ifdef KABCDAV_DEBUG
    log("setting debug carddav options");
    enableCarddavDebug(carddav_runtime);
#endif // KABCDAV_DEBUG

    log("running job");
    res = runJob(carddav_runtime);

    if (OK != res) {
        log("job failed");
        processError(carddav_runtime->error);
    }

    carddav_free_runtime_info(&carddav_runtime);

    // Signal done
    // 1000 is read, 1001 is write
    if (type() == 0) TQApplication::postEvent ( parent(), new TQEvent( static_cast<TQEvent::Type>(1000) ) );
    if (type() == 1) TQApplication::postEvent ( parent(), new TQEvent( static_cast<TQEvent::Type>(1001) ) );
}

// EOF ========================================================================

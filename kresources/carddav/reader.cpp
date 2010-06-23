/*=========================================================================
| KABCDAV
|--------------------------------------------------------------------------
| (c) 2010  Timothy Pearson
|
| This project is released under the GNU General Public License.
| Please see the file COPYING for more details.
|--------------------------------------------------------------------------
| Remote address book loading.
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

using namespace KABC;

/*=========================================================================
| METHODS
 ========================================================================*/

void CardDavReader::cleanJob() {
    CardDavJob::cleanJob();
    mData = "";
}

int CardDavReader::runJob(runtime_info* RT) {
    kdDebug() << "reader::run, url: " << url();

    response* result = carddav_get_response();
    CARDDAV_RESPONSE res = OK;

    kdDebug() << "getting all objects";
    if (getUseURI() == false)
        res = carddav_getall_object(result, std::string(url().ascii()).c_str(), RT);
    else
        res = carddav_getall_object_by_uri(result, std::string(url().ascii()).c_str(), RT);

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

    carddav_free_response(&result);

    return res;
}

// EOF ========================================================================

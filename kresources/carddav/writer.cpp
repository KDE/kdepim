/*=========================================================================
| KABCDAV
|--------------------------------------------------------------------------
| (c) 2010  Timothy Pearson
|
| This project is released under the GNU General Public License.
| Please see the file COPYING for more details.
|--------------------------------------------------------------------------
| Remote address book writing class.
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

// Use carddav_modify_object() function.
// If it's not set, a pair of carddav_delete_object/carddav_add_object
// is used for modifying objects.
// It's done, because, for some reason, Zimbra returns an error
// on carddav_modify_object.
//#define USE_CARDDAV_MODIFY

/*=========================================================================
| NAMESPACE
 ========================================================================*/

using namespace KABC;

/*=========================================================================
| METHODS
 ========================================================================*/

void CardDavWriter::cleanJob() {
    CardDavJob::cleanJob();
}

int CardDavWriter::runJob(runtime_info* RT) {
    kdDebug() << "writer::run, url: " << url() << "\n";

    int res = OK;

    kdDebug() << "pushing added objects";
    res = pushObjects(mAdded, carddav_add_object, OK, RT);
    if (OK == res) {
#ifdef USE_CARDDAV_MODIFY
        kdDebug() << "pushing changed objects";
        if (getUseURI() == false)
            res = pushObjects(mChanged, carddav_modify_object, OK, RT);
        else
            res = pushObjects(mChanged, carddav_modify_object_by_uri, OK, RT);
        if (OK == res) {
            kdDebug() << "pushing deleted objects";
            if (getUseURI() == false)
                res = pushObjects(mDeleted, carddav_delete_object, OK, RT);
            else
                res = pushObjects(mDeleted, carddav_delete_object_by_uri, OK, RT);
        }
#else // if USE_CARDDAV_MODIFY
        kdDebug() << "pushing changed objects (delete)";
        if (getUseURI() == false)
            res = pushObjects(mChanged, carddav_delete_object, OK, RT);
        else
            res = pushObjects(mChanged, carddav_delete_object_by_uri, OK, RT);
        if (OK == res) {
            kdDebug() << "pushing changed objects (add)";
            if (getUseURI() == false)
                res = pushObjects(mChanged, carddav_add_object, OK, RT);
            else
                res = pushObjects(mChanged, carddav_add_object, OK, RT);
            if (OK == res) {
                kdDebug() << "pushing deleted objects";
                if (getUseURI() == false)
                    res = pushObjects(mDeleted, carddav_delete_object, OK, RT);
                else
                    res = pushObjects(mDeleted, carddav_delete_object_by_uri, OK, RT);
            }
        }
#endif // if USE_CARDDAV_MODIFY
    }

    if (OK != res) {
        clearObjects();
    }

    return res;
}

// EOF ========================================================================

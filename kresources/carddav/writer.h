/*=========================================================================
| KCardDAV
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

#ifndef KCARDDAV_WRITER_H
#define KCARDDAV_WRITER_H

#include "job.h"

#include <string>
#include <tqstring.h>
#include <tqdatetime.h>

namespace KABC {

/*=========================================================================
| CLASS
 ========================================================================*/

/**
 * Calendar writer.
 */
class CardDavWriter : public CardDavJob {

public:

    /**
     * @param url URL to load.
     */
    CardDavWriter(const TQString& url = TQString()) :
        CardDavJob(url)
    {
        clearObjects();
    }

    /**
     * Sets the information about added incidences writer should send to server.
     * @param s icalendar-formatted string consists of all added incidences plus necessary calendar info.
     * May be an empty string, which means there is no added incidences to send.
     */
    void setAddedObjects(const TQString& s) {
        mAdded = s;
    }

    /**
     * Sets the information about changed incidences writer should send to server.
     * @param s icalendar-formatted string consists of all changed incidences plus necessary calendar info.
     * May be an empty string, which means there is no changed incidences to send.
     */
    void setChangedObjects(const TQString& s) {
        mChanged = s;
    }

    /**
     * Sets the information about deleted incidences writer should send to server.
     * @param s icalendar-formatted string consists of all deleted incidences plus necessary calendar info.
     * May be an empty string, which means there is no deleted incidences to send.
     */
    void setDeletedObjects(const TQString& s) {
        mDeleted = s;
    }

    /**
     * Clear all the information previously set.
     */
    void clearObjects() {
        setAddedObjects("");
        setChangedObjects("");
        setDeletedObjects("");
    }

protected:

    virtual int runJob(runtime_info* caldavRuntime);

    virtual void cleanJob();

    /// Just a wrapper above libcaldav functions.
    template<typename Operation>
    int pushObjects(const TQString& data, Operation op, int okCode, runtime_info* RT) {
        int r = okCode;
        if (!data.isNull() && !data.isEmpty()) {
            r = op(std::string(data.ascii()).c_str(), std::string(url().ascii()).c_str(), RT);
        }
        return r;
    }

private:

    TQString mAdded;
    TQString mChanged;
    TQString mDeleted;
};

} // namespace KABC

#endif // KCARDDAV_WRITER_H


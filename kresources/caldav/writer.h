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

#ifndef KCALDAV_WRITER_H
#define KCALDAV_WRITER_H

#include "job.h"

#include <string>
#include <qstring.h>
#include <qdatetime.h>

namespace KCal {

/*=========================================================================
| CLASS
 ========================================================================*/

/**
 * Calendar writer.
 */
class CalDavWriter : public CalDavJob {

public:

    /**
     * @param url URL to load.
     */
    CalDavWriter(const QString& url = QString()) :
        CalDavJob(url)
    {
        clearObjects();
    }

    /**
     * Sets the information about added incidences writer should send to server.
     * @param s icalendar-formatted string consists of all added incidences plus necessary calendar info.
     * May be an empty string, which means there is no added incidences to send.
     */
    void setAddedObjects(const QString& s) {
        mAdded = s;
    }

    /**
     * Sets the information about changed incidences writer should send to server.
     * @param s icalendar-formatted string consists of all changed incidences plus necessary calendar info.
     * May be an empty string, which means there is no changed incidences to send.
     */
    void setChangedObjects(const QString& s) {
        mChanged = s;
    }

    /**
     * Sets the information about deleted incidences writer should send to server.
     * @param s icalendar-formatted string consists of all deleted incidences plus necessary calendar info.
     * May be an empty string, which means there is no deleted incidences to send.
     */
    void setDeletedObjects(const QString& s) {
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
    int pushObjects(const QString& data, Operation op, int okCode, runtime_info* RT) {
        int r = okCode;
        if (!data.isNull() && !data.isEmpty()) {
            r = op(std::string(data.ascii()).c_str(), std::string(url().ascii()).c_str(), RT);
        }
        return r;
    }

private:

    QString mAdded;
    QString mChanged;
    QString mDeleted;
};

} // namespace KCal

#endif // KCALDAV_WRITER_H


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
#include <tqstring.h>
#include <tqdatetime.h>

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
    CalDavWriter(const TQString& url = TQString()) :
        CalDavJob(url)
    {
        clearObjects();
    }

    /**
     * Sets the information about added events writer should send to server.
     * @param s icalendar-formatted string consists of all added events plus necessary calendar info.
     * May be an empty string, which means there are no added events to send.
     */
    void setAddedObjects(const TQString& s) {
        mAdded = s;
    }

    /**
     * Sets the information about changed events writer should send to server.
     * @param s icalendar-formatted string consists of all changed events plus necessary calendar info.
     * May be an empty string, which means there are no changed events to send.
     */
    void setChangedObjects(const TQString& s) {
        mChanged = s;
    }

    /**
     * Sets the information about deleted events writer should send to server.
     * @param s icalendar-formatted string consists of all deleted events plus necessary calendar info.
     * May be an empty string, which means there are no deleted events to send.
     */
    void setDeletedObjects(const TQString& s) {
        mDeleted = s;
    }

    /**
     * Sets the information about added tasks writer should send to server.
     * @param s icalendar-formatted string consists of all added tasks plus necessary calendar info.
     * May be an empty string, which means there are no added tasks to send.
     */
    void setAddedTasksObjects(const TQString& s) {
        mTasksAdded = s;
    }

    /**
     * Sets the information about changed tasks writer should send to server.
     * @param s icalendar-formatted string consists of all changed tasks plus necessary calendar info.
     * May be an empty string, which means there are no changed tasks to send.
     */
    void setChangedTasksObjects(const TQString& s) {
        mTasksChanged = s;
    }

    /**
     * Sets the information about deleted tasks writer should send to server.
     * @param s icalendar-formatted string consists of all deleted tasks plus necessary calendar info.
     * May be an empty string, which means there are no deleted tasks to send.
     */
    void setDeletedTasksObjects(const TQString& s) {
        mTasksDeleted = s;
    }

    /**
     * Clear all the information previously set.
     */
    void clearObjects() {
        setAddedObjects("");
        setChangedObjects("");
        setDeletedObjects("");
        setAddedTasksObjects("");
        setChangedTasksObjects("");
        setDeletedTasksObjects("");
    }

protected:

    virtual int runJob(runtime_info* caldavRuntime);
    virtual int runTasksJob(runtime_info* caldavRuntime);

    virtual void cleanJob();
    virtual void cleanTasksJob();

    /// Just a wrapper above libcaldav event writing functions.
    template<typename Operation>
    int pushObjects(const TQString& data, Operation op, int okCode, runtime_info* RT) {
        int r = okCode;
        if (!data.isNull() && !data.isEmpty()) {
            r = op(std::string(data.ascii()).c_str(), std::string(url().ascii()).c_str(), RT);
        }
        return r;
    }

    /// Just a wrapper above libcaldav task writing functions.
    template<typename Operation>
    int pushTasksObjects(const TQString& data, Operation op, int okCode, runtime_info* RT) {
        int r = okCode;
        if (!data.isNull() && !data.isEmpty()) {
            r = op(std::string(data.ascii()).c_str(), std::string(tasksUrl().ascii()).c_str(), RT);
        }
        return r;
    }

private:

    TQString mAdded;
    TQString mChanged;
    TQString mDeleted;

    TQString mTasksAdded;
    TQString mTasksChanged;
    TQString mTasksDeleted;
};

} // namespace KCal

#endif // KCALDAV_WRITER_H


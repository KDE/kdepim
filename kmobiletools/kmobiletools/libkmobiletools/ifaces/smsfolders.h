/***************************************************************************
   Copyright (C) 2007 by Matthias Lechner <matthias@lmme.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KMOBILETOOLSIFACESSMSFOLDERS_H
#define KMOBILETOOLSIFACESSMSFOLDERS_H

#include <QtCore/QObject>
#include <QtCore/QList>

#include <libkmobiletools/kmobiletools_export.h>

/**
  * @TODO define and implement the SMSFolder object
  * it should at least contain an id, a name and a inbox/outbox flag
  *
  * note to myself: think about the edit case.. here we can allow only a name change
  * and we must deny access to all other information
  */


namespace KMobileTools {

namespace Ifaces {
class SMSFolder;

/**
    This interface provides methods to access the phone's sms folder structure

    @author Matthias Lechner <matthias@lmme.de>
*/
class KMOBILETOOLS_EXPORT SMSFolders {
public:
//public Q_SLOTS:
    /**
     * Fetches information about the phone's sms folders
     */
    virtual void fetchSMSFolders() = 0;

public:
    /**
     * Returns the list of the fetched sms folders
     *
     * @return the list of the fetched folders
     */
    virtual QList<SMSFolder> smsFolders() = 0;

    /**
     * Adds the given @p smsFolder on the phone
     *
     * @param smsFolder the folder to add
     */
    virtual void addSMSFolder( const SMSFolder& smsFolder ) = 0;

    /**
     * Replaces the @p oldFolder with the given @p newFolder
     *
     * @param oldFolder the folder to be replaced
     * @param newFolder the new folder
     */
    virtual void editSMSFolder( const SMSFolder& oldFolder, const SMSFolder& newFolder ) = 0;

    /**
     * Removes the given @p smsFolder from the phone
     *
     * @param smsFolder the folder to remove
     */
    virtual void removeSMSFolder( const SMSFolder& smsFolder ) = 0;

    virtual ~SMSFolders();

protected:
//Q_SIGNALS:
    /**
     * This signal is emitted when the sms folder information has been fetched from
     * the phone
     */
    virtual void smsFoldersFetched() = 0;

    /**
     * This signal is emitted when a sms folder has been added
     *
     * @p smsFolder the folder that was added
     */
    virtual void smsFolderAdded( SMSFolder smsFolder ) = 0;

    /**
     * This signal is emitted when a sms folder has been successfully replaced on the phone
     *
     * @p oldFolder the old folder
     * @p newFolder the new folder
     */
    virtual void smsFolderEdited( SMSFolder oldFolder, SMSFolder newFolder ) = 0;

    /**
     * This signal is emitted when a sms folder has been removed
     *
     * @p smsFolder the folder that was removed
     */
    virtual void smsFolderRemoved( SMSFolder smsFolder ) = 0;
};

}
}

Q_DECLARE_INTERFACE(KMobileTools::Ifaces::SMSFolders, "org.kde.KMobileTools.Ifaces.SMSFolders/0.1")


#endif

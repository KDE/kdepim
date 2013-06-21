/*
  Copyright 2010 Tobias Koenig <tokoe@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef ADDCONTACTJOB_H
#define ADDCONTACTJOB_H

#include "kdepim_export.h"

#include <kjob.h>

namespace Akonadi {
class Collection;
}

namespace KABC {
class Addressee;
}

namespace KPIM {

/**
 * @short A job to add a new contact to Akonadi.
 *
 * The job will check whether a contact with the given email address already
 * exists in Akonadi and adds it if it does not exist yet.
 */
class KDEPIM_EXPORT AddContactJob : public KJob
{
  Q_OBJECT

  public:
    /**
     * Creates a new add contact job.
     *
     * If the contact is not found, the user will be presented a dialog to
     * choose the address book where the new contact shall be stored.
     *
     * @param contact The contact to add.
     * @param parentWidget The widget that will be used as parent for dialog.
     * @param parent The parent object.
     */
    AddContactJob( const KABC::Addressee &contact, QWidget *parentWidget, QObject *parent = 0 );

    /**
     * Creates a new add contact job.
     *
     * @param contact The contact to add.
     * @param collection The address book collection where the contact shall be stored in.
     * @param parent The parent object.
     */
    AddContactJob( const KABC::Addressee &contact, const Akonadi::Collection &collection, QObject *parent = 0 );

    /**
     * Destroys the add email address job.
     */
    ~AddContactJob();

    /**
     * Starts the job.
     */
    virtual void start();

    void showMessageBox(bool b);

  private:
    //@cond PRIVATE
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void slotSearchDone( KJob* ) )
    Q_PRIVATE_SLOT( d, void slotAddContactDone( KJob* ) )
    //@endcond
};

}

#endif

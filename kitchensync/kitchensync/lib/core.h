/*
    This file is part of KitchenSync.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KSYNC_CORE_H
#define KSYNC_CORE_H

#include "manipulatorpart.h"
#include "profile.h"

#include <kdebug.h>

#include <qwidget.h>

namespace KSync
{

class ProfileManager;
class KonnectorManager;
class SyncUi;
class SyncAlgorithm;

enum KonnectorMode { KONNECTOR_ONLINE = 0, KONNECTOR_OFFLINE };

/**
 * The KitchenSync UI Shell
 * It's the MainWindow of the application. It'll load all parts
 * and do the basic communication between all parts
 */
class Core : public QWidget
{
   Q_OBJECT
  public:
    Core( QWidget *parent );
    ~Core();

    /**
     * @return the currently enabled Profile
     */
    virtual Profile currentProfile() const = 0;

    /**
     * @return access to the profilemanager
     * @FIXME make const pointer to const object
     */
    virtual ProfileManager *profileManager() const = 0;

    /**
     * @return a SyncUi
     */
    virtual SyncUi *syncUi() = 0;

    /**
     * @return the preferred syncAlgorithm of KitchenSync
     */
    virtual SyncAlgorithm *syncAlgorithm() = 0;

    /**
     * @return the all loaded ManipulatorParts
     */
    virtual const QPtrList<ManipulatorPart> parts() const = 0;

  signals:
    /**
     * This signal gets emitted whenever the Profile
     * is changed.
     * @param oldProfile the previously enabled profile
     */
    void profileChanged( const Profile &oldProfile );

    /**
     * signal emitted when progress from the konnectorProgress arrived
     * @param konnector pointer to Konnector object
     * @param prog the Progress
     */
    void konnectorProgress( Konnector *konnector , const Progress &prog );

    /**
     * @param konnector pointer to Konnector object
     * @param err the error
     */
    void konnectorError( Konnector *konnector, const Error &err );

    /**
     * Whenever the currently activated parts changed
     * @param newPart the newly activated part
     */
    void partChanged( ManipulatorPart *newPart );

    /**
     * progress coming from one part
     * @param part where the progress comes from, 0 if from MainWindow
     * @param prog The progress
     */
    void partProgress( ManipulatorPart *part, const Progress &prog );

    /**
     * error coming from one part
     * @param part where the error comes from, 0 if from MainWindow
     * @param err The error
     */
    void partError( ManipulatorPart *part, const Error &error );

    /**
     * emitted when ever sync starts
     */
    void startSync();

    /**
     * emitted when a part is asked to sync
     */
    void startSync( ManipulatorPart * );

    void syncProgress( ManipulatorPart *, int, int );
    /**
     * emitted when done with syncing
     */
    void doneSync();

    /**
     * emitted when one part is done with syncing
     */
    void doneSync( ManipulatorPart * );
};

}

#endif

/*
    This file is part of libkdepim.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#ifndef KABC_RESOURCECACHED_H
#define KABC_RESOURCECACHED_H

#include <kabc/resource.h>
#include <kdepimmacros.h>

#include <tqdatetime.h>
#include <tqtimer.h>

#include "libemailfunctions/idmapper.h"

namespace KABC {

class KDE_EXPORT ResourceCached : public Resource
{
  Q_OBJECT

  public:
    /**
      Reload policy.

      @see setReloadPolicy(), reloadPolicy()
    */
    enum { ReloadNever, ReloadOnStartup, ReloadInterval };
    /**
      Save policy.

      @see setSavePolicy(), savePolicy()
    */
    enum { SaveNever, SaveOnExit, SaveInterval, SaveDelayed, SaveAlways };

    ResourceCached( const KConfig* );
    ~ResourceCached();

    /**
      Set reload policy. This controls when the cache is refreshed.

      ReloadNever     never reload
      ReloadOnStartup reload when resource is started
      ReloadInterval  reload regularly after given interval
    */
    void setReloadPolicy( int policy );
    /**
      Return reload policy.

      @see setReloadPolicy()
    */
    int reloadPolicy() const;

    /**
      Set reload interval in minutes which is used when reload policy is
      ReloadInterval.
    */
    void setReloadInterval( int minutes );

    /**
      Return reload interval in minutes.
    */
    int reloadInterval() const;

    /**
      Set save policy. This controls when the cache is refreshed.

      SaveNever     never save
      SaveOnExit    save when resource is exited
      SaveInterval  save regularly after given interval
      SaveDelayed   save after small delay
      SaveAlways    save on every change
    */
    void setSavePolicy( int policy );
    /**
      Return save policy.

      @see setsavePolicy()
    */
    int savePolicy() const;

    /**
      Set save interval in minutes which is used when save policy is
      SaveInterval.
    */
    void setSaveInterval( int minutes );

    /**
      Return save interval in minutes.
    */
    int saveInterval() const;

    void setupSaveTimer();
    void setupReloadTimer();

   /**
     Reads the resource specific config from disk.
    */
   virtual void readConfig( KConfig *config );

    /**
      Writes the resource specific config to disk.
     */
    virtual void writeConfig( KConfig *config );

    /**
      Insert an addressee into the resource.
     */
    virtual void insertAddressee( const Addressee& );

    /**
      Removes an addressee from resource.
     */
    virtual void removeAddressee( const Addressee& addr );

    void loadCache();
    void saveCache();
    void clearCache();
    void cleanUpCache( const KABC::Addressee::List &list );

    /**
      Returns a reference to the id mapper.
     */
    KPIM::IdMapper& idMapper();

    bool hasChanges() const;
    void clearChanges();
    void clearChange( const KABC::Addressee& );
    void clearChange( const TQString& );

    KABC::Addressee::List addedAddressees() const;
    KABC::Addressee::List changedAddressees() const;
    KABC::Addressee::List deletedAddressees() const;

  protected:
    virtual TQString cacheFile() const;

    /**
      Functions for keeping the changes persistent.
     */
    virtual TQString changesCacheFile( const TQString& ) const;
    void loadChangesCache( TQMap<TQString, KABC::Addressee>&, const TQString& );
    void loadChangesCache();
    void saveChangesCache( const TQMap<TQString, KABC::Addressee>&, const TQString& );
    void saveChangesCache();

    void setIdMapperIdentifier();

  private:
    TQMap<TQString, KABC::Addressee> mAddedAddressees;
    TQMap<TQString, KABC::Addressee> mChangedAddressees;
    TQMap<TQString, KABC::Addressee> mDeletedAddressees;

    KPIM::IdMapper mIdMapper;

    class ResourceCachedPrivate;
    ResourceCachedPrivate *d;

    int mReloadPolicy;
    int mReloadInterval;
    TQTimer mKABCReloadTimer;
    bool mReloaded;

    int mSavePolicy;
    int mSaveInterval;
    TQTimer mKABCSaveTimer;

    TQDateTime mLastLoad;
    TQDateTime mLastSave;

  protected slots:
    void slotKABCReload();
    void slotKABCSave();
};

}

#endif

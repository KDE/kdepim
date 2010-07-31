/*
    This file is part of kdepim.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KPIM_IDMAPPER_H
#define KPIM_IDMAPPER_H

#include <tqmap.h>
#include <tqvariant.h>

#include <kdepimmacros.h>

namespace KPIM {

/**
    An Id Mapper maps Ids. What to or what for is not entirely
    clear, but maps have categories. This is probably an
    adjoint functor, since adjoint functors are everywhere.
*/
class KDE_EXPORT IdMapper
{
  public:
    /**
      Create Id mapper. You have to set path and identifier before you can call
      load() or save().
    */
    IdMapper();
    /**
      Create Id mapper. The path specifies the category of mapping, the
      identifier the concrete object.
      
      If you don't pass an identifier you have to set it before calling load()
      or save().
      
      The current implementation stores the data at
      $(KDEHOME)/share/apps/\<path\>/\<identifier\>.
    */
    IdMapper( const TQString &path, const TQString &identifier = TQString::null );
    /** Destructor. */
    ~IdMapper();

    /**
      Set id map path.
    */
    void setPath( const TQString &path );
    /**
      Return id map path.
    */
    TQString path() const { return mPath; }

    /**
      Set id map identifier.
    */
    void setIdentifier( const TQString &identifier );
    /**
      Return id map identifier.
    */
    TQString identifier() const { return mIdentifier; }

    /**
      Loads the map.
     */
    bool load();

    /**
      Saves the map.
     */
    bool save();

    /**
      Clears the map.
     */
    void clear();

    /**
      Stores the remote id for the given local id.
     */
    void setRemoteId( const TQString &localId, const TQString &remoteId );

    /**
      Removes the remote id.
     */
    void removeRemoteId( const TQString &remoteId );

    /**
      Returns the remote id of the given local id.
     */
    TQString remoteId( const TQString &localId ) const;

    /**
      Returns the local id for the given remote id.
     */
    TQString localId( const TQString &remoteId ) const;


    /**
     * Stores a fingerprint for an id which can be used to detect if 
     * the locally held version differs from what is on the server.
     * This can be a sequence number of an md5 hash depending on what
     * the server provides
     */
    void setFingerprint( const TQString &localId, const TQString &fingerprint );

    /**
     * Returns the fingerprint for the map.
     *
     * @todo Figure out if this applies to the last set fingerprint
     *       or if anything else can change it.
     */
    const TQString &fingerprint( const TQString &localId ) const;


    /**
     * Returns the entire map for the Id mapper.
     *
     * @todo Document what the map means.
     */
    TQMap<TQString, TQString> remoteIdMap() const;

    /**
     * Returns a string representation of the id pairs, that's usefull
     * for debugging.
     */
    TQString asString() const;

  protected:
    /**
     * Returns the filename this mapper is (or will be) stored in.
     */
    TQString filename();

  private:
    TQMap<TQString, TQVariant> mIdMap;
    TQMap<TQString, TQString> mFingerprintMap;

    TQString mPath;
    TQString mIdentifier;
};

}

#endif

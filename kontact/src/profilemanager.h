/*
    This file is part of KDE Kontact.

    Copyright (c) 2007 Frank Osterfeld <frank.osterfeld@kdemail.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef KONTACT_PROFILEMANAGER_H
#define KONTACT_PROFILEMANAGER_H

#include <tqmap.h>
#include <tqobject.h>
#include <tqstring.h>

template <class T> class QValueList;

namespace KIO {
    class Job;
}

namespace Kontact {

class Profile
{
    friend class ProfileManager;
public:
    Profile();

    explicit Profile( const TQString& id, bool isLocal = false );

    TQString id() const;

    TQString name() const;

    TQString description() const;

    bool isNull() const;

    void setName( const TQString& name );

    void setDescription( const TQString& description );

    bool operator==( const Kontact::Profile& other ) const;

    TQString saveLocation() const;

private: // ProfileManager only

    enum SetLocalMode {
        DoNotCopyProfileFiles,
        CopyProfileFiles
    };
    void setLocal( SetLocalMode mode );
    bool isLocal() const;
    void setOriginalLocation( const TQString& path );
    void setId( const TQString& id );

private:

    static void copyConfigFiles( const TQString& source, const TQString& dest );

    TQString localSaveLocation() const;

private:
    TQString m_id;
    TQString m_name;
    TQString m_description;
    bool m_local;
    TQString m_originalLocation;
};

class ProfileManager : public QObject
{
Q_OBJECT
public:
    enum ImportError {
        SuccessfulImport=0,
        NoValidProfile
    };

    enum ExportError {
        SuccessfulExport=0,
        DirectoryDoesNotExist,
        DirectoryNotWritable
    };

    static ProfileManager* self();

    ~ProfileManager();

    Kontact::Profile profileById( const TQString& id ) const;

    bool addProfile( const Kontact::Profile& profile, bool syncConfig = true );

    void removeProfile( const Kontact::Profile& profile );

    void removeProfile( const TQString& id );

    void updateProfile( const Kontact::Profile& profile );

    void loadProfile( const TQString& id );

    void saveToProfile( const TQString& id );

    TQValueList<Kontact::Profile> profiles() const;

    ExportError exportProfileToDirectory( const TQString& id, const TQString& path );

    ImportError importProfileFromDirectory( const TQString& path );

    TQString generateNewId() const;

signals:
    void profileAdded( const TQString& id );

    void profileRemoved( const TQString& id );

    void profileUpdated( const TQString& id );

    void profileLoaded( const TQString& id );

    void saveToProfileRequested( const TQString& id );

    void profileImportFinished( ImportError status );

private:
    static ProfileManager* m_self;

    static Kontact::Profile readFromConfiguration( const TQString& configFile, bool isLocal );

    explicit ProfileManager( TQObject* parent = 0 );

    void readConfig();

    void writeConfig() const;

    void writeProfileConfig( const Kontact::Profile& profile ) const;

private:
    TQMap<TQString, Kontact::Profile> m_profiles;
};

}

#endif // KONTACT_PROFILEMANAGER_H

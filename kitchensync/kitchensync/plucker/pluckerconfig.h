/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Holger Hans Peter Freyther <freyther@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef KS_PLUCKER_CONFIG_H
#define KS_PLUCKER_CONFIG_H

#include <kstaticdeleter.h>
#include <qstringlist.h>

class KConfig;
namespace KSPlucker {
class PluckerConfig 
{
    template<class> friend class KStaticDeleter;
    PluckerConfig();
public:
    ~PluckerConfig();
    static PluckerConfig* self();

//@{
    QStringList pluckerFiles()const;
    QString     javaPath()const;
    QString     pluckerPath()const;
    QStringList konnectorIds()const;

    void setPluckerFiles( const QStringList& path );
    void setJavaPath( const QString& );
    void setPluckerPath( const QString& );
    void setKonnectorIds( const QStringList& );
//@}



//@{
    void load(const QString& profileUid);
    void save(const QString&);
//@}

private:
    QStringList m_paths;
    QStringList m_konnectors;
    QString m_javaPath;
    QString m_pluckerPath;
};
}


#endif

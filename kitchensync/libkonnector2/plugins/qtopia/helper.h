/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2003 Holger Freyther <freyther@kde.org>

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
#ifndef OpieHelperBase_H
#define OpieHelperBase_H

#include <sys/types.h>
#include <stdlib.h>
#include <time.h>

#include <qdatetime.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>

#include <ktempfile.h>

#include <kontainer.h>
#include <syncer.h>

#include <idhelper.h>

#include "extramap.h"
#include "categoryedit.h"

namespace OpieHelper {
    class Device;
    class Base {
    public:
        Base( CategoryEdit* edit =0,
              KSync::KonnectorUIDHelper* helper = 0,
              const QString &tz = QString::null,
              bool metaSyncing = FALSE, Device* d = 0);
        virtual ~Base();
    protected:
        // from tt GPLed
        time_t toUTC( const QDateTime& dt );
        QDateTime fromUTC( time_t time );
        // off tt code

	/** returns a new KTempFile */
        KTempFile* file();
	/** generates a new id */
        int newId();
        CategoryEdit* edit() { return m_edit; };
        KSync::KonnectorUIDHelper* helper() { return m_helper; };
        bool isMetaSyncingEnabled()const;
        void setMetaSyncingEnabled(bool meta);

        // returns a ; separated list of real ids
        // will also add the value m_kde2opie
        QString categoriesToNumber( const QStringList &categories,
                                    const QString &app= QString::null );
        // convience method
        QString konnectorId( const QString &appName,  const QString &uid );
        QString kdeId( const QString &appName, const QString &uid );

	const Device* device();

	CategoryEdit *m_edit;
        KSync::KonnectorUIDHelper *m_helper;
        Kontainer::ValueList m_kde2opie;
        bool m_metaSyncing : 1;
        QString m_tz;
    private:
        Device* m_device;
        class BasePrivate;
        BasePrivate *baseD;
    };

    /** escapes a whole string */
    QString escape(const QString&);

}


#endif

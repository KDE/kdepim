/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>

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
#ifndef KSYNC_MAN_PART_SERVICE_H
#define KSYNC_MAN_PART_SERVICE_H

#include <qvaluelist.h>
#include <qstring.h>

#include <kservice.h>

namespace KSync {
   /**
    * a ManPartServive saves a converted
    * KService::Ptr of a ManipulatorPart
    * @see KService::Ptr
    */
    class ManPartService {
    public:
        /**
         * creates an Empty Service
         */
        typedef QValueList<ManPartService> ValueList;
        ManPartService();
        ManPartService( const KService::Ptr& service );
        ManPartService( const ManPartService& );
        ~ManPartService();
        bool operator==( const ManPartService& );
        bool operator==( const ManPartService& )const;
        QString name()const;
        QString comment()const;
        QString libname()const;
        QString icon() const;

        void setName(const QString & );
        void setComment( const QString& comment );
        void setLibname( const QString& );
        void setIcon( const QString& );

        ManPartService &operator=( const ManPartService&);
    private:
        QString m_name;
        QString m_comment;
        QString m_icon;
        QString m_lib;
    };
};


#endif

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
#ifndef CategoryEdit_H
#define CategoryEdit_H

#include <qmap.h>
#include <qstring.h>
#include <qvaluelist.h>
#include <qstringlist.h>

#include "opiecategories.h"

namespace OpieHelper {

    class CategoryEdit {
    public:
        CategoryEdit();
        CategoryEdit(const QString &fileName);
        ~CategoryEdit();

        void save(const QString&) const;
        int addCategory( const QString &name, int id = 0 );
        int addCategory(const QString &appName,  const QString &name,  int id = 0);
        void parse( const QString &fileName );

        QString categoryById(const QString &id, const QString &app )const;
        QStringList categoriesByIds( const QStringList& ids,  const QString& app );

        void clear();
        QValueList<OpieCategories> categories()const {  return m_categories; };
    private:
        /**
         * this function will be used internally to update the kde categories...
         */
        void updateKDE( const QString& app,  const QStringList& categories );
        QMap<int, bool> ids; // from tt Qtopia::UidGen
        QValueList<OpieCategories> m_categories;
    };
};


#endif

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
#ifndef KONTAINER_H
#define KONTAINER_H

#include <qstring.h>
#include <qvaluelist.h>

class Kontainer
{
  public:
    /**
      Convinience typedef
    */
    typedef QValueList<Kontainer> ValueList;

    friend bool operator== ( const Kontainer &a ,  const Kontainer &b );
    Kontainer(const QString& = QString::null,
              const QString& = QString::null );
    Kontainer(const Kontainer & );

    ~Kontainer();

    QString first()const;
    QString second()const;
    Kontainer &operator=( const Kontainer& );

  private:
    class KontainerPrivate;
    KontainerPrivate *d;
    QString m_first;
    QString m_second;
};

#endif

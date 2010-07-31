/*
    This file is part of libkdepim.

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
#ifndef KPIM_DESIGNERFIELDS_H
#define KPIM_DESIGNERFIELDS_H

#include <klocale.h>

#include <tqmap.h>
#include <tqpair.h>
#include <tqstringlist.h>

#include <kdepimmacros.h>

namespace KPIM {

class KDE_EXPORT DesignerFields : public QWidget
{
    Q_OBJECT
  public:
    DesignerFields( const TQString &uiFile, TQWidget *parent,
      const char *name = 0 );

    class Storage
    {
      public:
        virtual ~Storage() {}
      
        virtual TQStringList keys() = 0;
        virtual TQString read( const TQString &key ) = 0;
        virtual void write( const TQString &key, const TQString &value ) = 0;
    };

    void load( Storage * );
    void save( Storage * );

    void setReadOnly( bool readOnly );

    TQString identifier() const;
    TQString title() const;

  signals:
    void modified();

  private:
    void initGUI( const TQString& );

    TQMap<TQString, TQWidget *> mWidgets;
    TQValueList<TQWidget *> mDisabledWidgets;
    TQString mTitle;
    TQString mIdentifier;
};

}

#endif

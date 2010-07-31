/*
    This file is part of Kung.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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

#ifndef BOOL_INPUTFIELD_H
#define BOOL_INPUTFIELD_H

#include <tqobject.h>

#include "inputfield.h"

class QCheckBox;

class BoolInputField : public SimpleInputField
{
  Q_OBJECT

  public:
    BoolInputField( const TQString &name, const Schema::SimpleType *type );

    virtual void setXMLData( const TQDomElement &element );
    virtual void xmlData( TQDomDocument &document, TQDomElement &parent );

    virtual void setData( const TQString &data );
    virtual TQString data() const;

    virtual TQWidget *createWidget( TQWidget *parent );

  private slots:
    void inputChanged( bool );

  private:
    TQCheckBox *mInputWidget;
    bool mValue;
};

#endif

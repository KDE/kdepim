/*
    This file is part of KAddressbook.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef ADVANCEDCUSTOMFIELDS_H
#define ADVANCEDCUSTOMFIELDS_H

#include <klocale.h>

#include <qmap.h>
#include <qpair.h>
#include <qstringlist.h>

#include <libkdepim/designerfields.h>

#include "contacteditorwidget.h"

class AdvancedCustomFields : public KAB::ContactEditorWidget
{
  Q_OBJECT

  public:
    AdvancedCustomFields( const QString &uiFile, KABC::AddressBook *ab,
                          QWidget *parent, const char *name = 0 );

    void loadContact( KABC::Addressee *addr );
    void storeContact( KABC::Addressee *addr );
    void setReadOnly( bool readOnly );

    QString pageIdentifier() const;
    QString pageTitle() const;

  private:
    void initGUI( const QString& );

    KPIM::DesignerFields *mFields;
};

#endif

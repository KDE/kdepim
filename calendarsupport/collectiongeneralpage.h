/*
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net,
  Author: Tobias Koenig <tokoe@kdab.com>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/
#ifndef CALENDARSUPPORT_COLLECTIONGENERALPAGE_H
#define CALENDARSUPPORT_COLLECTIONGENERALPAGE_H

#include "calendarsupport_export.h"

#include <CollectionPropertiesPage>

class KLineEdit;
class KIconButton;
class QCheckBox;

namespace CalendarSupport {

class CALENDARSUPPORT_EXPORT CollectionGeneralPage : public Akonadi::CollectionPropertiesPage
{
  Q_OBJECT

  public:
    explicit CollectionGeneralPage( QWidget *parent = 0 );
    ~CollectionGeneralPage();

    void load( const Akonadi::Collection &collection );
    void save( Akonadi::Collection &collection );

  private:
    QCheckBox *mBlockAlarmsCheckBox;
    KLineEdit *mNameEdit;
    QCheckBox *mIconCheckBox;
    KIconButton *mIconButton;
};

AKONADI_COLLECTION_PROPERTIES_PAGE_FACTORY( CollectionGeneralPageFactory, CollectionGeneralPage )

}

#endif

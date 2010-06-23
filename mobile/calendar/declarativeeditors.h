/*
    Copyright (c) 2010 Bertjan Broeksema <b.broeksema@home.nl>

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

#ifndef DECLRARATIVEEDITOR_H
#define DECLRARATIVEEDITOR_H

#include <Akonadi/CollectionComboBox>

#include <incidenceeditors/incidenceeditor-ng/incidencegeneral.h>
#include <incidenceeditors/incidenceeditor-ng/incidencedatetime.h>

#include "declarativewidgetbase.h"
#include "incidenceview.h"

class DCollectionCombo
  : public DeclarativeWidgetBase<Akonadi::CollectionComboBox,
                                 IncidenceView,
                                 &IncidenceView::setCollectionCombo>
{
  Q_OBJECT
  public:
    explicit DCollectionCombo( QDeclarativeItem *parent = 0 );
};

class MobileIncidenceGeneral : public IncidenceEditorsNG::IncidenceGeneral
{
  Q_OBJECT
  public:
    explicit MobileIncidenceGeneral( QWidget *parent = 0 );
};

class DIEGeneral
  : public DeclarativeWidgetBase<MobileIncidenceGeneral,
                                 IncidenceView,
                                 &IncidenceView::setGeneralEditor>
{
  Q_OBJECT
  public:
    explicit DIEGeneral( QDeclarativeItem *parent = 0 );
};

class MobileIncidenceDateTime : public IncidenceEditorsNG::IncidenceDateTime
{
  Q_OBJECT
  public:
    explicit MobileIncidenceDateTime( QWidget *parent = 0 );
};

class DIEDateTime
  : public DeclarativeWidgetBase<MobileIncidenceDateTime,
                                 IncidenceView,
                                 &IncidenceView::setDateTimeEditor>
{
  Q_OBJECT
  public:
    explicit DIEDateTime( QDeclarativeItem *parent = 0 );
};

#endif

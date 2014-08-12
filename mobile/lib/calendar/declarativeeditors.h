/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>

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

#ifndef DECLARATIVEEDITORS_H
#define DECLARATIVEEDITORS_H

#include "declarativewidgetbase.h"
#include "incidenceview.h"

#include <AkonadiWidgets/CollectionComboBox>

#include <QStackedWidget>

namespace Ui
{
  class EventOrTodoDesktop;
  class EventOrTodoMore;
}

/// DIE == DeclarativeIncidenceEditor

class DCollectionCombo
  : public DeclarativeWidgetBase<Akonadi::CollectionComboBox,
                                 IncidenceView,
                                 &IncidenceView::setCollectionCombo>
{
  Q_OBJECT
  public:
    explicit DCollectionCombo( QGraphicsItem *parent = 0 );
};

class MobileIncidenceGeneral : public QWidget
{
  Q_OBJECT
  public:
    explicit MobileIncidenceGeneral( QWidget *parent = 0 );

    ~MobileIncidenceGeneral();

  public:
    Ui::EventOrTodoDesktop *mUi;
};

class DIEGeneral
  : public DeclarativeWidgetBase<MobileIncidenceGeneral,
                                 IncidenceView,
                                 &IncidenceView::setGeneralEditor>
{
  Q_OBJECT
  public:
    explicit DIEGeneral( QGraphicsItem *parent = 0 );

  private Q_SLOTS:
    void hack();
};

class MobileIncidenceMore : public QStackedWidget
{
  Q_OBJECT
  public:
    explicit MobileIncidenceMore( QWidget *parent = 0 );

    ~MobileIncidenceMore();

  public:
    Ui::EventOrTodoMore *mUi;
};

class DIEMore
  : public DeclarativeWidgetBase<MobileIncidenceMore,
                                 IncidenceView,
                                 &IncidenceView::setMoreEditor>
{
  Q_OBJECT

  Q_PROPERTY( int currentIndex READ currentIndex WRITE setCurrentIndex )

  public:
    explicit DIEMore( QGraphicsItem *parent = 0 );

    int currentIndex() const;

  public Q_SLOTS:
    void setCurrentIndex( int index );
};

#endif

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

#ifndef INCIDENCEVIEW_H
#define INCIDENCEVIEW_H

#include <KCal/Incidence>

#include <incidenceeditors/incidenceeditor-ng/combinedincidenceeditor.h>

#include "kdeclarativefullscreenview.h"

namespace Akonadi {
class CollectionComboBox;
}

namespace IncidenceEditorsNG {
class IncidenceGeneralEditor;
class IncidenceDateTimeEditor;
}


class IncidenceView : public KDeclarativeFullScreenView
{
  Q_OBJECT;
  public:
    explicit IncidenceView( QWidget* parent = 0 );
    ~IncidenceView();

    void setCollectionCombo( Akonadi::CollectionComboBox * );
    void setDateTimeEditor( IncidenceEditorsNG::IncidenceDateTimeEditor * );
    void setGeneralEditor( IncidenceEditorsNG::IncidenceGeneralEditor * );

  public slots:
    void save();   /// Ok clicked in the user interface
    void cancel(); /// Cancel clicked in the user interface

    
  private:
    Akonadi::CollectionComboBox *mCollectionCombo;
    KCal::Incidence::Ptr mEvent;
    IncidenceEditorsNG::CombinedIncidenceEditor *mEditor;
};

#endif // INCIDENCEVIEW_H

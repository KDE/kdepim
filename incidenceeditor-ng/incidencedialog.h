/*
  Copyright (C) 2010 Bertjan Broeksema <broeksema@kde.org>
  Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#ifndef INCIDENCEEDITOR_INCIDENCEDIALOG_H
#define INCIDENCEEDITOR_INCIDENCEDIALOG_H

#include "incidenceeditors-ng_export.h"

#include <KDialog>

namespace Akonadi {
  class Collection;
  class Item;
}

namespace IncidenceEditorNG {

class INCIDENCEEDITORS_NG_EXPORT IncidenceDialog : public KDialog
{
  Q_OBJECT

  public:
    explicit IncidenceDialog( QWidget *parent = 0, Qt::WindowFlags flags = 0 );
    virtual ~IncidenceDialog();

    /**
     * Loads the @param item into the dialog.
     *
     * To create a new Incidence pass an invalid item with either an
     * KCalCore::Event:Ptr or a KCalCore::Todo:Ptr set as payload. Note: When the
     * item is invalid, i.e. it has an invalid id, a valid payload <em>must</em>
     * be set.
     *
     * When the item has is valid this method will fetch the payload when this is
     * not already set.
     *
     * @param activeDate Sets the active date to use for loaded incidences. This
     * is used in particular for recurring incidences.
     */
    virtual void load( const Akonadi::Item &item, const QDate &activeDate = QDate() ) = 0;

    /**
     * Sets the Collection combobox to @param collection. Selects the first collection
     * when @param collection is invalid.
     */
    virtual void selectCollection( const Akonadi::Collection &collection ) = 0;

    /**
     * Indicates whether or not the loaded incidence must be treated as a counter
     * proposal. By default incidences are <em>not</em> treated as counter
     * proposals.
     */
    virtual void setIsCounterProposal( bool isCounterProposal ) = 0;

    /**
      Returns the object that will receive all key events.
    */
    virtual QObject *typeAheadReceiver() const = 0;
};

}

#endif

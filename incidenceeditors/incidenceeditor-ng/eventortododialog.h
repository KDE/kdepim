/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#ifndef EVENTORTODODIALOG_H
#define EVENTORTODODIALOG_H

#include <KDialog>

#include "incidenceeditors-ng_export.h"

class KJob;

namespace Akonadi {
class Item;
class CollectionComboBox;
}

namespace IncidenceEditorsNG {

class EventOrTodoDialogPrivate;

class INCIDENCEEDITORS_NG_EXPORT EventOrTodoDialog : public KDialog
{
  Q_OBJECT;
  public:
    EventOrTodoDialog( QWidget *parent = 0 );
    ~EventOrTodoDialog();

    /**
     * Loads the @param item into the dialog.
     *
     * To create a new Incidence pass an invalid item with either an
     * KCal::Event:Ptr or a KCal::Todo:Ptr set as payload.
     *
     * When the item has is valid it will fetch the payload when this is not
     * set.
     */
    void load( const Akonadi::Item &item );

  protected Q_SLOTS:
    /** Override default behiavor of KDialog. */
    void slotButtonClicked( int buttonCode );

  private:
    EventOrTodoDialogPrivate *d_ptr;
    Q_DECLARE_PRIVATE( EventOrTodoDialog );
    Q_DISABLE_COPY( EventOrTodoDialog );

    Q_PRIVATE_SLOT(d_ptr, void itemChanged( const Akonadi::Item&, const QSet<QByteArray>& ))
    Q_PRIVATE_SLOT(d_ptr, void itemFetchResult(KJob*))
    Q_PRIVATE_SLOT(d_ptr, void modifyFinished(KJob*))
    Q_PRIVATE_SLOT(d_ptr, void save())
    Q_PRIVATE_SLOT(d_ptr, void updateAttachmentCount(int))
    Q_PRIVATE_SLOT(d_ptr, void updateButtonStatus(bool))
};

}

#endif // EVENTORTODODIALOG_H

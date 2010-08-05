/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
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

#ifndef EVENTORTODODIALOGNG_H
#define EVENTORTODODIALOGNG_H

#include <KDialog>

#include "editoritemmanager.h"
#include "incidenceeditors-ng_export.h"

namespace Akonadi {
class Item;
}

namespace IncidenceEditorsNG {

class EventOrTodoDialogPrivate;

class INCIDENCEEDITORS_NG_EXPORT EventOrTodoDialog : public KDialog
{
  Q_OBJECT
public:
  EventOrTodoDialog( QWidget *parent = 0, Qt::WFlags flags = 0 );
  ~EventOrTodoDialog();

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
   */
  void load( const Akonadi::Item &item );

  /**
   * Sets the Collection combobox to @param collection.
   */
  void selectCollection( const Akonadi::Collection &collection );

protected Q_SLOTS:
  virtual void slotButtonClicked( int button );

private:
  EventOrTodoDialogPrivate * const d_ptr;
  Q_DECLARE_PRIVATE( EventOrTodoDialog )
  Q_DISABLE_COPY( EventOrTodoDialog )

  Q_PRIVATE_SLOT(d_ptr, void handleAlarmCountChange(int))
  Q_PRIVATE_SLOT(d_ptr, void handleItemSaveFinish(Akonadi::EditorItemManager::SaveAction))
  Q_PRIVATE_SLOT(d_ptr, void handleItemSaveFail(Akonadi::EditorItemManager::SaveAction, QString))
  Q_PRIVATE_SLOT(d_ptr, void handleRecurrenceChange(int))
  Q_PRIVATE_SLOT(d_ptr, void loadTemplate(QString))
  Q_PRIVATE_SLOT(d_ptr, void saveTemplate(QString))
  Q_PRIVATE_SLOT(d_ptr, void storeTemplatesInConfig(QStringList))
  Q_PRIVATE_SLOT(d_ptr, void updateAttachmentCount(int))
  Q_PRIVATE_SLOT(d_ptr, void updateAttendeeCount(int))
  Q_PRIVATE_SLOT(d_ptr, void updateButtonStatus(bool))
};

}

#endif // EVENTORTODODIALOGNG_H

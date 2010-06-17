#ifndef EVENTORTODODIALOGNG_H
#define EVENTORTODODIALOGNG_H

#include <KDialog>

#include "incidenceeditors-ng_export.h"

namespace Akonadi {
class Item;
}

class EventOrTodoDialogNGPrivate;

namespace IncidenceEditorsNG {

class INCIDENCEEDITORS_NG_EXPORT EventOrTodoDialogNG : public KDialog
{
  Q_OBJECT
public:
  EventOrTodoDialogNG();
  ~EventOrTodoDialogNG();

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

private:
  EventOrTodoDialogNGPrivate * const d_ptr;
  Q_DECLARE_PRIVATE( EventOrTodoDialogNG )
  Q_DISABLE_COPY( EventOrTodoDialogNG )

  Q_PRIVATE_SLOT(d_ptr, void updateButtonStatus(bool))
};

}

#endif // EVENTORTODODIALOGNG_H

#ifndef RECURRENCEPRESETS_H
#define RECURRENCEPRESETS_H

#include <QtCore/QBitArray>

class QString;
class QStringList;

namespace KCal {
class Recurrence;
}

class KDateTime;

namespace IncidenceEditorsNG {

namespace RecurrencePresets {

  /**
   * Returns the availble presets.
   */
  QStringList availablePresets();

  /**
   * Returns a recurrence preset for given name. The name <em>must</em> be one
   * of availablePresets().
   *
   * Note: The caller takes ownership over the pointer.
   */
  KCal::Recurrence* preset( const QString &name, const KDateTime &start);

} // RecurrencePresets

} // IncidenceEditorsNG

#endif // RECURRENCEPRESETS_H

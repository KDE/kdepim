#ifndef ABBROWSERCONDUITCONFIG_H
#define ABBROWSERCONDUITCONFIG_H
#include "abbrowserConduitConfig_base.h"

class KConfig;
class AbbrowserConduitConfig : public AbbrowserConduitConfig_base
    { 
      Q_OBJECT
      
    public:
      AbbrowserConduitConfig( KConfig &c, QWidget* parent = 0,
			      const char* name = 0, WFlags fl = 0 );
      ~AbbrowserConduitConfig();

      int commitChanges(KConfig& c);
      
      static const QString SMART_MERGE_ENTRY; /** bool */
      /** int (EConflictResolution) */
      static const QString CONFLICT_RESOLV_ENTRY;
      static const QString PILOT_OTHER_MAP_ENTRY; /* QString */
      static const QString PILOT_STREET_TYPE_ENTRY; /* QString */
      static const QString PILOT_FAX_TYPE_ENTRY; /* QString */
      static const QString CLOSE_ABBROWSER_ON_EXIT; /* bool */
      static const QString FIRST_TIME_SYNCING; /* bool */
    };

#endif // ABBROWSERCONDUITCONFIG_H

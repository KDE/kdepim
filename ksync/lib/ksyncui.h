#ifndef KSYNCUI_H
#define KSYNCUI_H
// $Id$

class KSyncEntry;

/**
  @short Syncing conflict resolution user interface.
  @author Cornelius Schumacher
  @see KSyncer
  
  This class provides the abstract interface to a conflict resolution user
  interface. It is needed for cases, when a syncing process cannot resolve
  conflicts automatically. This is the case, when the same data entry has been
  changed in different data sets in an incompatible way.
  
  This class has to be implemented by a concrete subclass, which provides the
  actual user interface. While a GUI implementation, which provides interactive
  conflict resolution, is the most common implementation, there might also be
  use for a non-GUI or even non-interactive user interface.
*/
class KSyncUi
{
  public:
    KSyncUi();
    virtual ~KSyncUi();
    
    /**
      Deconflict two conflicting @ref KSyncEntry objects. Returns the entry,
      which has been chosen by the user to take precedence over the other.
      
      The default implementation always returns 0, which should be interpreted
      to not sync the entries at all. Reimplement this function in a subclass to
      provide a more useful implementation to @ref KSyncer.
    */
    virtual KSyncEntry *deconflict(KSyncEntry *syncEntry,KSyncEntry *target);
};

#endif

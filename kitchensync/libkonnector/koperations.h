#ifndef koperations_h
#define koperations_h

#include <qvaluelist.h>

// nothing funny here yet
/**
 *  KOperations is responsible to save filesystem
 *  operations like copy, move, delete
 */
namespace KSync {
 
class KOperations {
 public:
   /**
    * Typedef for convenience
    */
    typedef QValueList<KOperations> ValueList;
	
	KOperations();
    ~KOperations();
 private:
    class KOperationsPrivate;
    KOperationsPrivate *d;
};
};
#endif

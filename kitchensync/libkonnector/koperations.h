#ifndef koperations_h
#define koperations_h

#include <qvaluelist.h>

// nothing funny here yet
/**
 *  KOperations is responsible to save filesystem
 *  operations like copy, move, delete
 */
class KOperations {
 public:
   /**
    * Typedef for convinience
    */
    typedef QValueList<KOperations> List;
	
	KOperations();
    ~KOperations();
 private:
    class KOperationsPrivate;
    KOperationsPrivate *d;
};

#endif

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
    KOperations();
    ~KOperations();
 private:
    class KOperationsPrivate;
    KOperationsPrivate *d;
};

/**
 * Typedef for convinience
 */
typedef QValueList<KOperations> KOperationsList;

#endif

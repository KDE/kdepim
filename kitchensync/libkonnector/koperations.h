#ifndef koperations_h
#define koperations_h

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

#endif

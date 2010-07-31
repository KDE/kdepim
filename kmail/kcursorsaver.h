#ifndef kcursorsaver_h
#define kcursorsaver_h

#include <tqcursor.h>
#include <tqapplication.h>

/**
 * @short sets a cursor and makes sure it's restored on destruction
 * Create a KCursorSaver object when you want to set the cursor.
 * As soon as it gets out of scope, it will restore the original
 * cursor.
 */
class KCursorSaver : public Qt
{
public:
    /// constructor taking TQCursor shapes
    KCursorSaver(Qt::CursorShape shape) {
        TQApplication::setOverrideCursor( TQCursor(shape) );
        inited = true;
    }

    /// copy constructor. The right side won't restore the cursor
    KCursorSaver( const KCursorSaver &rhs ) {
        *this = rhs;
    }

    /// restore the cursor
    ~KCursorSaver() {
        if (inited)
            TQApplication::restoreOverrideCursor();
    }

    /// call this to explitly restore the cursor
    inline void restoreCursor(void) {
        TQApplication::restoreOverrideCursor();
        inited = false;
    }

protected:
    void operator=( const KCursorSaver &rhs ) {
        inited = rhs.inited;
        rhs.inited = false;
    }

private:
    mutable bool inited;
};

/**
 * convenience functions
 */
namespace KBusyPtr {
    inline KCursorSaver idle() {
        return KCursorSaver(TQCursor::ArrowCursor);
    }
    inline KCursorSaver busy() {
        return KCursorSaver(TQCursor::WaitCursor);
    }
}

#endif /*kbusyptr_h_*/

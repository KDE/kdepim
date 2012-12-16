/*
  Copyright 2010 Olivier Trichet <nive@nivalis.org>

  Permission to use, copy, modify, and distribute this software
  and its documentation for any purpose and without fee is hereby
  granted, provided that the above copyright notice appear in all
  copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

#ifndef KNODE_SCOPEDCURSOROVERRIDE_H
#define KNODE_SCOPEDCURSOROVERRIDE_H

#include <QApplication>

namespace KNode {
namespace Utilities {

/**
  This object change the application cursor to a given type and
  then restore the previous cursor when it goes out of scope.
  @see QApplication::setOverrideCursor()
*/
class ScopedCursorOverride
{
  public:
    /**
      Constructor: change the cursor shape to @p shape.
    */
    explicit ScopedCursorOverride( Qt::CursorShape shape )
      : restored( false )
    {
      QApplication::setOverrideCursor( QCursor( shape ) );
    }

    /**
      Destructor: restore the previous cursor shape.
    */
    ~ScopedCursorOverride()
    {
      restore();
    }

    /**
      Restore the previous cursor shape.
    */
    void restore()
    {
      if ( !restored ) {
        restored = true;
        QApplication::restoreOverrideCursor();
      }
    }

  private:
    bool restored;
};

} // namespace Utilities
} // namespace KNode

#endif // KNODE_SCOPEDCURSOROVERRIDE_H

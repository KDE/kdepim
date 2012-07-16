/*
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Andras Mantia <andras@kdab.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#ifndef MESSAGEVIEWER_MINIMUMCOMBOBOX_H
#define MESSAGEVIEWER_MINIMUMCOMBOBOX_H

#include <kcombobox.h>
#include "messageviewer_export.h"

namespace MessageViewer {

  /** @short A KComboBox, which minimum size hint can be really small */
  class MESSAGEVIEWER_EXPORT MinimumComboBox: public KComboBox {
    Q_OBJECT
  public:
      explicit MinimumComboBox(QWidget* parent = 0) : KComboBox( parent ) {}
      virtual QSize minimumSizeHint() const { return QSize( 50, KComboBox::minimumSizeHint().height() ); }
  };

}

#endif

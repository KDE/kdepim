/*
  Copyright 2011 Volker Krause <vkrause@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef KPIM_NEPOMUKWARNING_H
#define KPIM_NEPOMUKWARNING_H

#include "kdepim_export.h"

#include <KDE/KMessageWidget>

namespace KPIM {

/**
 * Inline warning message for windows/dialogs that semi-optionally depend on Nepomuk.
 * Just add this to the layout at the top, it'll automatically show itself when necessary.
 */
class KDEPIM_EXPORT NepomukWarning : public KMessageWidget
{
  Q_OBJECT
  public:
    explicit NepomukWarning( QWidget *parent = 0 );

  private slots:
    void configure();
};

}

#endif

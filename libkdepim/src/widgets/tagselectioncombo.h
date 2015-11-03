/*
  Copyright (c) 2014 Christian Mollekopf <mollekopf@kolabsys.com>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef TAGSELECTIONCOMBO_H
#define TAGSELECTIONCOMBO_H

#include "kdepim_export.h"
#include <KComboBox>
#include "kcheckcombobox.h"

namespace KPIM
{

class KDEPIM_EXPORT TagSelectionCombo : public KPIM::KCheckComboBox
{
    Q_OBJECT
public:
    explicit TagSelectionCombo(QWidget *parent = Q_NULLPTR);
};

class KDEPIM_EXPORT TagCombo : public KComboBox
{
    Q_OBJECT
public:
    explicit TagCombo(QWidget *parent = Q_NULLPTR);
};

}

#endif // TAGSELECTIONCOMBO_H

/*
    KNode, the KDE newsreader
    Copyright (c) 2005 Volker Krause <volker.krause@rwth-aachen.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNODE_CSSHELPER_H
#define KNODE_CSSHELPER_H

#include <libkdepim/csshelper.h>

namespace KNode {

class CSSHelper : public KPIM::CSSHelper
{
  public:
    CSSHelper( const QPaintDeviceMetrics &pdm );

};

}

#endif

// -*- C++ -*-
/* ownkhexedit.h		KPilot
**
** Copyright (C) 2003 by Dan Pilone
** Written 2003 by Reinhold Kainhofer
**
** This is a dialog window that is used to edit a single address record.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#ifndef KPILOT_OWNKHEXEDIT_H
#define KPILOT_OWNKHEXEDIT_H

#ifdef USE_KHEXEDIT
#include <khexedit/khexedit.h>
#include <khexedit/kwrappingrobuffer.h>
#else

class QWidget;
#include <qtextedit.h>

namespace KHE {

class KWrappingROBuffer {
public: 
	KWrappingROBuffer(char*l, int len) {}
//	~KWrappingROBuffer(){};
};
		
class KHexEdit : public QTextEdit {
public:
  KHexEdit( void *Buffer = 0, QWidget *Parent = 0, const char *Name = 0, WFlags F = 0 ) :
		QTextEdit( i18n("Here will be the raw data once the KHE::KHexEdit is made public"), "", Parent) {};
	void setDataBuffer(KWrappingROBuffer*b)  {};
};

}
#endif

#endif





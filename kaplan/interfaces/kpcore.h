/*
    This file is part of Kaplan
    Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
    Copyright (c) 2002 Daniel Molkentin <molkentin@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// $Id:$

#ifndef KP_CORE_H
#define KP_CORE_H


#include <qstring.h>
#include <qobject.h>

#include <kparts/mainwindow.h>
#include <kparts/part.h>

class KAction;

namespace Kaplan
{


	class Core : public KParts::MainWindow
	{
	protected:

		Core(QWidget *parentWidget = 0, const char *name = 0);
	public:
		virtual ~Core();

		virtual void addMainEntry(QString text, QString icon, QObject *reveiver, const char *slot) = 0;

		virtual void addPart(KParts::Part *part) = 0;

                virtual void showPart(KParts::Part *part) = 0;

		virtual void insertNewAction(KAction *action) = 0;

	};


};

#endif
// vim: ts=4 sw=4 et


/*
	Empath - Mailer for KDE

	Copyright (C) 1998 Rik Hemsley rik@kde.org

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

#ifndef RMM_RPARAMETERLIST_H
#define RMM_RPARAMETERLIST_H

#include <qstring.h>
#include <qlist.h>

#include <RMM_Defines.h>
#include <RMM_Parameter.h>
#include <RMM_HeaderBody.h>

typedef QListIterator<RParameter> RParameterListIterator;

/**
 * @short Simple encapsulation of a list of RParameter, which is also an
 * RHeaderBody.
 */
class RParameterList : public QList<RParameter>, public RHeaderBody {

	public:

		RParameterList();
		RParameterList(const RParameterList &);
		RParameterList(const QCString &);
		RParameterList & operator = (const RParameterList &);
		RParameterList & operator = (const QCString &);

		virtual ~RParameterList();

		void parse();
		void assemble();
		void createDefault();

		const char * className() const { return "RParameterList"; }
};

#endif //RPARAMETERLIST_H


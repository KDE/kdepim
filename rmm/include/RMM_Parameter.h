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

#ifndef RMM_PARAMETER_H
#define RMM_PARAMETER_H

#include <qstring.h>

#include <RMM_MessageComponent.h>
#include <RMM_Defines.h>

/**
 * An RParameter consists of an attribute, value pair.
 * It is used in RParameterList, for example when looking at an RCte field.
 */
class RParameter : public RMessageComponent {

	public:

		RParameter();
		RParameter(const RParameter & p);
		RParameter(const QCString & s) : RMessageComponent(s) { }

		virtual ~RParameter();

		const RParameter & operator = (const RParameter &);

		void parse();
		void assemble();

		const QCString & attribute() const { return attribute_; }
		const QCString & value() const { return value_; }

		void setAttribute(const QCString & attribute) { attribute_ = attribute; }
		void setValue(const QCString & value) { value_ = value; }
		
		void createDefault();
		
		const char * className() const { return "RParameter"; }

	private:

		QCString attribute_;
		QCString value_;
};

#endif

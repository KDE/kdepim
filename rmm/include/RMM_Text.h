/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rikkus@postmaster.co.uk
	
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

#ifndef RMM_RTEXT_H
#define RMM_RTEXT_H

#include <RMM_Defines.h>
#include <RMM_HeaderBody.h>

class RText : public RHeaderBody {

	public:

		RText();
		RText(const RText & rText);
		RText(const QString & s) : RHeaderBody(s) { }
		const RText & operator = (const RText & rText);

		virtual ~RText();

		void parse();
		void assemble();

		bool isValid() const;

		const char * className() const { return "RText"; }

		void createDefault();

	private:

		bool isValid_;
};

#endif //RTEXT_H

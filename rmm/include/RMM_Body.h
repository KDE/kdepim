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

#ifndef RMM_BODY_H
#define RMM_BODY_H

#include <qstring.h>
#include <qlist.h>

#include <RMM_BodyPart.h>
#include <RMM_Entity.h>
#include <RMM_MessageComponent.h>
#include <RMM_Defines.h>
#include <RMM_ContentType.h>
#include <RMM_Cte.h>

typedef QList<RBodyPart> RBodyPartList;

class RMessage;
class REntity;

class RBody : public QList<RBodyPart>, public RMessageComponent {

	public:

		RBody();
		RBody(const RBody & body);
		RBody(const QString & s) : RMessageComponent(s) { }

		virtual ~RBody();

		const RBody & operator = (const RBody & body);

		void parse();
		void assemble();
		void createDefault();

		QString		firstPlainBodyPart() { return ""; }
		int			numberOfParts() const;
		void		addPart(RBodyPart * bp);
		void		removePart(RBodyPart * part);
		RBodyPart	* part(int index);

		const char * className() const { return "RBody"; }

	private:

		QList<RBodyPart>	partList_;
		
		QString 			strRep_;

		QString				boundary_;
		QString				preamble_;
		QString				epilogue_;
		
		RContentType		contentType_;
		RCte				cte_;
};

#endif


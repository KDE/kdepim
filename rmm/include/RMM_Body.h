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

class RMessage;
class REntity;

/**
 * @short Container for one or more RBodyPart(s).
 * The RBody class contains one or more RBodyPart(s). Asking an RBody to parse
 * itself will result in the string representation being used to create these
 * RBodyPart(s). Assembling will recreate the string representation and where
 * necessary boundaries will be added, together with the preamble and epilogue.
 */
class RBody : public RMessageComponent {

	public:

		RBody();
		RBody(const RBody &);
		RBody(const QCString & s) : RMessageComponent(s) { }

		virtual ~RBody();

		RBody & operator = (const RBody &);

		void parse();
		void assemble();
		void createDefault();

		QCString	firstPlainBodyPart();
		int			numberOfParts();
		void		addPart(RBodyPart * bp);
		void		removePart(RBodyPart * part);
		RBodyPart	part(int index);
		
		void		setBoundary(const QCString & s);
		void		setContentType(RContentType & t);
		void		setCTE(RCte &);
		void 		setMultiPart(bool);

		const char * className() const { return "RBody"; }

	private:

		bool				isMultiPart_;

		RBodyPartList		partList_;
		
		QCString 			strRep_;

		QCString			boundary_;
		QCString			preamble_;
		QCString			epilogue_;
		
		RContentType		contentType_;
		RCte				cte_;
};

#endif


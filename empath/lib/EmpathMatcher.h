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

#ifndef EMPATHMATCHER_H
#define EMPATHMATCHER_H

#include "RMM_Message.h"
#include "EmpathMatcher.h"
#include "EmpathDefines.h"
#include "EmpathEnum.h"
#include "EmpathURL.h"

class EmpathMatcher
{
	public:
		
		EmpathMatcher();
		virtual ~EmpathMatcher();
		void load(const QString & parentName, Q_UINT32 id);
		void save(const QString & parentName, Q_UINT32 id);

		bool match(const EmpathURL &);
		QString description() const;
		
		const char * className() const { return "EmpathMatcher"; }
		
		MatchExprType type() const { return type_; }
		void setType(MatchExprType t) { type_ = t; }

		Q_UINT32 size() { return size_; }
		const QString & matchHeader() { return matchHeader_; }
		const QString & matchExpr() { return matchExpr_; }
		
		void setSize(Q_UINT32 s) { size_ = s; }
		void setMatchHeader(const QString & s) { matchHeader_ = s; }
		void setMatchExpr(const QString & s) { matchExpr_ = s; }
		
	private:
		
		MatchExprType type_;
		Q_UINT32 size_;
		QString matchHeader_;
		QString matchExpr_;
};

#endif


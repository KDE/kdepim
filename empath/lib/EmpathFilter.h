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

#ifndef EMPATHFILTER_H
#define EMPATHFILTER_H

// Qt includes
#include <qobject.h>
#include <qlist.h>

// Local includes
#include "RMM_MessageID.h"
#include "EmpathMatcher.h"
#include "EmpathFilterEventHandler.h"

typedef QListIterator<EmpathMatcher> EmpathMatcherListIterator;

class EmpathFilter : public QObject
{
	Q_OBJECT

	public:
		
		EmpathFilter();

		virtual ~EmpathFilter();
		void load(Q_UINT32 id);
		void save();
		
		void filter(const EmpathURL & source);

		void addMatchExpr(EmpathMatcher * matcher);
		void setId(Q_UINT32 id);

		Q_UINT32	id() const;
		QString		description() const;
		QString		actionDescription() const;
		void		setEventHandler(EmpathFilterEventHandler *);
		void		setURL(const EmpathURL & url);
		EmpathURL	url() const;

		QList<EmpathMatcher> *		matchExprList();
		EmpathFilterEventHandler *	eventHandler();
		
		void		setPriority(Q_UINT32 priority)
		{ priority_ = priority; }
		
		Q_UINT32	priority()
		{ return priority_; }
		
	private:

		bool match(const EmpathURL & id);
		void loadMatchExpr(Q_UINT32 matchExprID);
		void loadEventHandler();

		Q_UINT32 id_;
		Q_UINT32 priority_;
		
		EmpathURL					url_;
		QList<EmpathMatcher>		matchExprs_;
		EmpathFilterEventHandler	* fEventHandler_;
};

#endif // EMPATHFILTER_H


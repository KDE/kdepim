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

// Qt includes
#include <qregexp.h>

// KDE includes
#include <klocale.h>
#include <kconfig.h>
#include <kapp.h>

// Local includes
#include "Empath.h"
#include "EmpathDefines.h"
#include "EmpathMatcher.h"
#include "EmpathConfig.h"

EmpathMatcher::EmpathMatcher()
{
	empathDebug("ctor");
}

EmpathMatcher::~EmpathMatcher()
{
	empathDebug("dtor");
}

	void
EmpathMatcher::save(const QString & parentid, Q_UINT32 id)
{
	empathDebug("save(" + parentid + ", " + QString().setNum(id) + ") called");	
	
	KConfig * c = kapp->getConfig();
	
	c->setGroup(EmpathConfig::GROUP_EXPR +
		parentid +
		"_" +
		QString().setNum(id));
	
	c->writeEntry(EmpathConfig::KEY_MATCH_EXPR_TYPE, type_);
	
	switch (type_) {
		
		case Size:
			c->writeEntry(EmpathConfig::KEY_MATCH_SIZE, size_);
			break;
			
		case BodyExpr:
			c->writeEntry(EmpathConfig::KEY_MATCH_EXPR, matchExpr_);
			break;
			
		case HeaderExpr:
			c->writeEntry(EmpathConfig::KEY_MATCH_HEADER, matchHeader_);
			c->writeEntry(EmpathConfig::KEY_MATCH_EXPR, matchExpr_);
			break;
			
		case HasAttachments:
		case AnyMessage:
		default:
			break;
	}
	c->sync();
}

	void
EmpathMatcher::load(const QString & parentName, Q_UINT32 id)
{
	empathDebug("load(" + parentName + ", " +
		QString().setNum(id) + ") called");
	
	KConfig * c = kapp->getConfig();
	
	c->setGroup(EmpathConfig::GROUP_EXPR + parentName + "_" +
		QString().setNum(id));
	
	MatchExprType t =
		(MatchExprType)
		(c->readNumEntry(EmpathConfig::KEY_MATCH_EXPR_TYPE));
	
	empathDebug("I'm of type " + QString().setNum((int)t));
	
	setType(t);
	
	switch (t) {

		case Size:
			size_ = c->readNumEntry(EmpathConfig::KEY_MATCH_SIZE);
			break;

		case BodyExpr:
			matchExpr_ = c->readEntry(EmpathConfig::KEY_MATCH_EXPR);
			break;

		case HeaderExpr:
			matchExpr_ = c->readEntry(EmpathConfig::KEY_MATCH_EXPR);
			matchHeader_ = c->readEntry(EmpathConfig::KEY_MATCH_HEADER);
			break;

		case HasAttachments:
		case AnyMessage:
			break;

		default:
			// FIXME: ERROR ! We're supposed to have written the type of the
			// matcher to the config !
			return;
			break;
	}
}


	bool
EmpathMatcher::match(const EmpathURL & id)
{

	switch (type_) {
		
		case Size:
			{
				empathDebug("Matching message by size > " +
					QString().setNum(size_));

				RMessage * m(empath->message(id));
				if (m == 0) return false;

				RMessage message(*m);
				
				Q_UINT32 sizeOfMessage = message.size();

				empathDebug("size of message is " +
					QString().setNum(sizeOfMessage));

				// Size * 1024 as it's specified in Kb.
				return (sizeOfMessage > (size_ * 1024));
			}
			break;
			
		case BodyExpr:
			{
				empathDebug("Matching message by body expr \"" + matchExpr_ + "\"");
				RMessage * m(empath->message(id));
				if (m == 0) return false;
				
				RMessage message(*m);
				
				QString s; // FIXME -- = m->firstPlainBodyPart ?
				
				QRegExp r(matchExpr_);
				if (!r.isValid()) return false;
				return (s.find(r) != -1);
			}
			break;
			
		case HeaderExpr:
			{
				empathDebug("Matching message by header expr \"" + matchExpr_ + "\"");
				
				RMessage * m(empath->message(id));
				if (m == 0) return false;
				
				RMessage message(*m);
				
				QString s = message.envelope().asString();
				QRegExp r(matchExpr_);
				if (!r.isValid()) return false;
				return (s.find(r) != -1);
			}
			break;
			
		case HasAttachments:	
			empathDebug("Matching message by attachments");
			{
				RMessage * m(empath->message(id));
				if (m == 0) return false;
				
				RMessage message(*m);
				
				return (message.type() == RBodyPart::Mime);
			}
			break;
			
		case AnyMessage:
			empathDebug("I match any message. Matched !");
			return true;
			break;
			
		default:
			return false;
			break;
	}

	return false;
}
	
	QString
EmpathMatcher::description() const
{
	QString desc;

	switch (type_) {
		
		case Size:
			desc = i18n("Message is larger than");
			desc += " ";
			desc += QString().setNum(size_);
			desc += "Kb";	
			break;
			
		case BodyExpr:
			desc = i18n("Expression");
			desc += " '";
			desc += matchExpr_;
			desc += "' ";
			desc += i18n("in message body");
			break;
			
		case HeaderExpr:
			desc = i18n("Expression");
			desc += " '";
			desc += matchExpr_;
			desc += "' ";
			desc += i18n("in message header");
			desc += " '";
			desc += matchHeader_;
			desc += "'";
			break;
			
		case HasAttachments:
			desc = i18n("Message has attachments");
			break;
			
		case AnyMessage:
			desc = i18n("Any message");
			break;
			
		default:
			break;
	}
	
	return desc;
}
	

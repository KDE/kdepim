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

// Implementation note: This was in separate classes but I think the extra hassle
// to manage them was too much of a pain in the arse.

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
EmpathMatcher::save(Q_UINT32 parentid, Q_UINT32 id)
{
	empathDebug("save(" + QString().setNum(parentid) + ", " + QString().setNum(id) + ") called");	
	
	KConfig * c = kapp->getConfig();
	
	c->setGroup(GROUP_EXPR +
		QString().setNum(parentid) +
		"_" +
		QString().setNum(id));
	
	c->writeEntry(KEY_MATCH_EXPR_TYPE, type_);
	
	switch (type_) {
		
		case Size:
			c->writeEntry(KEY_MATCH_SIZE, size_);
			break;
			
		case BodyExpr:
			c->writeEntry(KEY_MATCH_EXPR, matchExpr_);
			break;
			
		case HeaderExpr:
			c->writeEntry(KEY_MATCH_HEADER, matchHeader_);
			c->writeEntry(KEY_MATCH_EXPR, matchExpr_);
			break;
			
		case HasAttachments:
		case AnyMessage:
		default:
			break;
	}
	c->sync();
}

	void
EmpathMatcher::load(Q_UINT32 parentid, Q_UINT32 id)
{
	empathDebug("load(" + QString().setNum(parentid) + ", " +
		QString().setNum(id) + ") called");
	
	KConfig * c = kapp->getConfig();
	
	c->setGroup(GROUP_EXPR +
		QString().setNum(parentid) +
		"_" +
		QString().setNum(id));
	
	MatchExprType t = (MatchExprType)(c->readNumEntry(KEY_MATCH_EXPR_TYPE));
	empathDebug("I'm of type " + QString().setNum((int)t));
	
	setType(t);
	
	switch (t) {

		case Size:
			size_ = c->readNumEntry(KEY_MATCH_SIZE);
			break;

		case BodyExpr:
			matchExpr_ = c->readEntry(KEY_MATCH_EXPR);
			break;

		case HeaderExpr:
			matchExpr_ = c->readEntry(KEY_MATCH_EXPR);
			matchHeader_ = c->readEntry(KEY_MATCH_HEADER);
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

				Q_UINT32 sizeOfMessage =
					empath->sizeOfMessage(id);

				empathDebug("size of message is " +
					QString().setNum(sizeOfMessage));

				// Size * 1024 as it's specified in Kb.
				if (sizeOfMessage > (size_ * 1024)) {
					empathDebug("Matched");
					return true;
				}
			}
			break;
			
		case BodyExpr:
			{
				empathDebug("Matching message by body expr \"" + matchExpr_ + "\"");
				QString s =
					empath->plainBodyOfMessage(id);
				QRegExp r(matchExpr_);
				if (!r.isValid()) return false;
				return (s.find(r) != -1);
			}
			break;
			
		case HeaderExpr:
			{
				empathDebug("Matching message by header expr \"" + matchExpr_ + "\"");
				REnvelope * e = empath->envelopeOfMessage(id);
				if (e == 0) return false;
				
				QString s = e->asString();
				QRegExp r(matchExpr_);
				if (!r.isValid()) return false;
				return (s.find(r) != -1);
			}
			break;
			
		case HasAttachments:	
			empathDebug("Matching message by attachments");
			if (empath->typeOfMessage(id) ==
				RMessage::MimeMessage)
			return true;
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
	

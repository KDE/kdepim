/*
	libvcard - vCard parsing library for vCard version 3.0

	Copyright (C) 1999 Rik Hemsley rik@kde.org
	
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

#include <qcstring.h>
#include <qstrlist.h>

#include <AdrParam.h>
#include <AgentParam.h>
#include <DateParam.h>
#include <EmailParam.h>
#include <ImageParam.h>
#include <SourceParam.h>
#include <TelParam.h>
#include <TextBinParam.h>
#include <TextParam.h>

#include <AdrValue.h>
#include <AgentValue.h>
#include <DateValue.h>
#include <ImageValue.h>
#include <TextValue.h>
#include <TextBinValue.h>
#include <LangValue.h>
#include <NValue.h>
#include <URIValue.h>
#include <SoundValue.h>
#include <ClassValue.h>
#include <FloatValue.h>
#include <OrgValue.h>
#include <TelValue.h>
#include <TextListValue.h>
#include <UTCValue.h>

#include <RToken.h>
#include <ContentLine.h>

#include <Entity.h>
#include <VCardEnum.h>
#include <VCardDefines.h>

using namespace VCARD;

ContentLine::ContentLine()
	:	Entity(),
		value_(0)
{
}

ContentLine::ContentLine(const ContentLine & x)
	:	Entity(x),
		paramList_	(x.paramList_),
		value_(x.value_)
{
}

ContentLine::ContentLine(const QCString & s)
	:	Entity(s),
		value_(0)
{
}

	ContentLine &
ContentLine::operator = (ContentLine & x)
{
	if (*this == x) return *this;
	
	paramList_ = x.paramList();
	value_ = x.value_;

	Entity::operator = (x);
	return *this;
}

	ContentLine &
ContentLine::operator = (const QCString & s)
{
	Entity::operator = (s);
	delete value_;
	value_ = 0;
	return *this;
}

	bool
ContentLine::operator == (ContentLine & x)
{
	x.parse();
	
	QListIterator<Param> it(x.paramList());
	
	if (!paramList_.find(it.current()))
		return false;

	return true;
}

ContentLine::~ContentLine()
{
	delete value_;
	value_ = 0;
}

	void
ContentLine::_parse()
{
	vDebug("parse");
	int split = strRep_.find(':');
	
	if (split == -1) { // invalid content line
		vDebug("No ':'");
		return;
	}
	
	QCString firstPart(strRep_.left(split));
	QCString valuePart(strRep_.mid(split + 1));
	
	split = firstPart.find('.');
	
	if (split != -1) {
		group_		= firstPart.left(split);
		firstPart	= firstPart.mid(split + 1);
	}
	
	vDebug("Group == " + group_);
	vDebug("firstPart == " + firstPart);
	vDebug("valuePart == " + valuePart);
	
	// Now we have the group, the name and param list together and the value.
	
	QStrList l;
	
	RTokenise(firstPart, ";", l);
	
	if (l.count() == 0) {// invalid - no name !
		vDebug("No name for this content line !");
		return;
	}
	
	name_ = l.at(0);

	// Now we have the name, so the rest of 'l' is the params.
	// Remove the name part.
	l.remove(0u);
	
	entityType_	= ParamNameToEntityType(name_);
	paramType_	= EntityTypeToParamType(entityType_);
	
	unsigned int i = 0;
	
	// For each parameter, create a new parameter of the correct type.

	QStrListIterator it(l);
	
	for (; it.current(); ++it, i++) {

		Param * p(0);
	
		switch (paramType_) {
		
			case ParamDate:		p = new DateParam;		break;
			case ParamAgent:	p = new AgentParam;		break;
			case ParamEmail:	p = new EmailParam;		break;
			case ParamImage:	p = new ImageParam;		break;
			case ParamTextBin:	p = new TextBinParam;	break;
			case ParamSource:	p = new SourceParam;	break;
			case ParamMailer:
			case ParamAddrText:
			default:			p = new TextParam;		break;
		}
		
		*p = l.at(i);
		p->parse();
		paramList_.append(p);
	}

	// Create a new value of the correct type.

	valueType_ = EntityTypeToValueType(entityType_);
	
	switch (valueType_) {
		
		case ValueSound:	value_ = new SoundValue;	break;
		case ValueAgent:	value_ = new AgentValue;	break;
		case ValueAddress:	value_ = new AdrValue;		break;
		case ValueTel:		value_ = new TelValue;		break;
		case ValueTextBin:	value_ = new TextBinValue;	break;
		case ValueOrg:		value_ = new OrgValue;		break;
		case ValueN:		value_ = new NValue;		break;
		case ValueUTC:		value_ = new UTCValue;		break;
		case ValueURI:		value_ = new URIValue;		break;
		case ValueClass:	value_ = new ClassValue;	break;
		case ValueFloat:	value_ = new FloatValue;	break;
		case ValueImage:	value_ = new ImageValue;	break;
		case ValueDate:		value_ = new DateValue;		break;
		case ValueTextList:	value_ = new TextListValue;	break;
		case ValueText:
		case ValueUnknown:
		default:			value_ = new TextValue;	break;
	}
	
	*value_ = valuePart;
}

	void
ContentLine::_assemble()
{
	vDebug("Assemble - my name is \"" + name_ + "\"");
	strRep_.truncate(0);
	
	if (!group_.isEmpty())
		strRep_ += group_ + '.';
	
	strRep_ += name_;

	vDebug("Adding parameters");
	ParamListIterator it(paramList_);
	
	for (; it.current(); ++it)
		strRep_ += ";" + it.current()->asString();
	
	vDebug("Adding value");
	if (value_ != 0)
		strRep_ += ":" + value_->asString();
}


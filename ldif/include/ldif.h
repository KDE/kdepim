/*
	libldif - LDAP LDIF parsing library

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
#include <qlist.h>

#ifndef LDIF_H
#define LDIF_H

#ifndef NDEBUG
#	include <iostream.h>
#	ifdef __GNUG__
#		define lDebug(a) cerr << className() << "::" << __FUNCTION__ << \
		"(" << __LINE__ << "): " << QCString((a)).data() << endl;
#	else
#       define lDebug(a) cerr << className() << ": " << QCString((a)).data() \
		<< endl;
#	endif
#else
#	define lDebug(a)
#endif


namespace LDIF {

class Entity
{
	public:
		
		Entity()
			:	parsed_		(false),
				assembled_	(true)
		{
			// empty
		}
		
		Entity(const Entity & e)
			:	strRep_		(e.strRep_),
				parsed_		(e.parsed_),
				assembled_	(e.assembled_)
		{
			// empty
		}
		
		Entity(const QCString & s)
			:	strRep_		(s),
				parsed_		(false),
				assembled_	(true)
		{
			// empty
		}

		Entity & operator = (Entity & e)
		{
			if (this == &e) return *this;

			strRep_		= e.strRep_;
			parsed_		= e.parsed_;
			assembled_	= e.assembled_;

			return *this;
		}

		Entity & operator = (const QCString & s)
		{
			strRep_		= s;
			parsed_		= false;
			assembled_	= true;
	
			return *this;
		}

		
		bool operator == (Entity & e)
		{ return asString() == e.asString(); }

		bool operator != (Entity & e)
		{ return !(*this == e); }

		virtual bool operator == (const QCString & s)
		{ return asString() == s; }

		virtual bool operator != (const QCString & s)
		{ return !(*this == s); }
		
		virtual ~Entity()
		{
			// empty
		}
		
		QCString asString()
		{ assemble(); return strRep_; }
		
		void parse()
		{ if (!parsed_) _parse(); parsed_ = true; assembled_ = false; }

		void assemble()
		{ if (assembled_) return; parse(); _assemble(); assembled_ = true; }
		
		virtual void _parse() = 0;
		virtual void _assemble() = 0;
	
	protected:
		
		QCString strRep_;
		bool parsed_;
		bool assembled_;
};

class LDAPString  : public Entity
{
	
#include "LDAPString-generated.h"
		
	public:
		
		QCString octets()
		{ parse(); return octets_; }
		
		void setOctets(const QCString & s)
		{ parse(); octets_ = s; }
		
	private:
		
		QCString octets_;
};

typedef LDAPString AttributeDescription;

class AttrTypeAndValue : public Entity
{
	
#include "AttrTypeAndValue-generated.h"
	
	public:
		
		QCString type()
		{ parse(); return type_; }
	
		QCString value()
		{ parse(); return value_; }
		
	private:
		
		QCString type_;
		QCString value_;
};

typedef QList<AttrTypeAndValue> AttrTypeAndValueList;
typedef QListIterator<AttrTypeAndValue> AttrTypeAndValueIterator;

class NameComponent : public Entity
{
	
#include "NameComponent-generated.h"
	
	public:
		
		AttrTypeAndValueList attrTypeAndValueList()
		{ parse(); return attrTypeAndValueList_; }
		
		void setAttrTypeAndValueList (const AttrTypeAndValueList & l)
		{ parse(); attrTypeAndValueList_ = l; }
		
	private:
		
		AttrTypeAndValueList attrTypeAndValueList_;
};

typedef QList<NameComponent> NameComponentList;
typedef QListIterator<NameComponent> NameComponentIterator;

class Dn : public Entity
{
	
#include "Dn-generated.h"
	
	public:
		
		bool encoded()
		{ parse(); return encoded_; }
	
		void setEncoded(bool b)
		{ parse(); encoded_ = b; }
		
		NameComponentList nameComponentList()
		{ parse(); return nameComponentList_; }
		
		void setNameComponentList(const NameComponentList & l)
		{ parse(); nameComponentList_ = l; }
		
	private:
		
		bool encoded_;
		NameComponentList nameComponentList_;
};

class DnSpec : public Entity
{
	
#include "DnSpec-generated.h"
	
	public:
	
		Dn dn()
		{ parse(); return dn_; }
		
		void setDn(Dn & dn)
		{ parse(); dn_ = dn; }
	
	private:
		
		Dn dn_;
};

class ValueSpec : public Entity
{
	
#include "ValueSpec-generated.h"
	
	public:
	
		enum ValueType {
			Plain,
			Base64,
			URL
		};
		
		ValueType valueType()
		{ parse(); return valueType_; }
		
		void setValueType(ValueType t)
		{ parse(); valueType_ = t; }
		
		QCString value()
		{ parse(); return value_; }
		
		void setValue(const QCString & s)
		{ parse(); value_ = s; }
		
	private:
		
		ValueType	valueType_;
		QCString	value_;
};

class AttrValSpec : public Entity
{
	
#include "AttrValSpec-generated.h"

	public:
		
		AttributeDescription attributeDescription()
		{ parse(); return attributeDescription_; }
		
		void setAttributeDescription(AttributeDescription & d)
		{ parse(); attributeDescription_ = d; }
		
		ValueSpec valueSpec()
		{ parse(); return valueSpec_; }
		
		void setValueSpec(ValueSpec & s)
		{ parse(); valueSpec_ = s; }
		
	private:
		
		AttributeDescription attributeDescription_;
		ValueSpec valueSpec_;
};

typedef QList<AttrValSpec> AttrValSpecList;
typedef QListIterator<AttrValSpec> AttrValSpecIterator;

class VersionSpec : public Entity
{
	
#include "VersionSpec-generated.h"
	
	public:
		
		int number()
		{ parse(); return number_; }
		
		void setNumber(int i)
		{ parse(); number_ = i; }
	
	private:
		
		int number_;
};

class LdifAttrValRec : public Entity
{
	
#include "LdifAttrValRec-generated.h"

	public:
		
		DnSpec dnSpec()
		{ parse(); return dnSpec_; }
		
		void setDnSpec(DnSpec & s)
		{ parse(); dnSpec_ = s; }
		
		AttrValSpecList attrValSpecList()
		{ parse(); return attrValSpecList_; }
		
		void setAttrValSpecList(AttrValSpecList & l)
		{ parse(); attrValSpecList_ = l; }
		
	private:
		
		DnSpec dnSpec_;
		AttrValSpecList attrValSpecList_;
};

typedef QList<LdifAttrValRec> LdifAttrValRecList;
typedef QListIterator<LdifAttrValRec> LdifAttrValRecIterator;


class LdifContent : public Entity
{
	
#include "LdifContent-generated.h"

	public:
		
		VersionSpec versionSpec()
		{ parse(); return versionSpec_; }
		
		void setVersionSpec(VersionSpec & s)
		{ parse(); versionSpec_ = s; }
		
		LdifAttrValRecList attrValRecList()
		{ parse(); return attrValRecList_; }
		
		void setAttrValRecList(LdifAttrValRecList & l)
		{ parse(); attrValRecList_ = l; }
	
	private:
		
		VersionSpec versionSpec_;
		LdifAttrValRecList attrValRecList_;
};

}

#endif

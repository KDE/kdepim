#include <qstring.h>
#include <qlist.h>

#include "VCardEntity.h"

#include "../include/AddressBook.h"
#include "../include/Entry.h"
#include "../include/Enum.h"
#include "../include/Field.h"
#include "../include/Value.h"

namespace KAB {
extern "C" {
int import(const char *, KAB::AddressBook *);
}


int import(const char * _str, KAB::AddressBook * ab) 
{
	QCString str(_str);

	Q_UINT32 imported = 0;
	
	VCARD::VCardEntity v(str);
	
	VCARD::VCardList cardList = v.cardList();
	
	VCARD::VCardListIterator it(cardList);
	
	for (; it.current(); ++it) {
		
		Entry e("CARD" + QString().setNum(imported));

		QList<VCARD::ContentLine>
			contentLineList = it.current()->contentLineList();
		
		QListIterator<VCARD::ContentLine> cit(contentLineList);
		
		for (; cit.current(); ++cit) {
			
			QString group		= cit.current()->group();
			QString name		= cit.current()->name();
			VCARD::ParamList p	= cit.current()->paramList();
			VCARD::EntityType t	= cit.current()->entityType();
			VCARD::Value * v	= cit.current()->value();
			
			QString n = (group.isEmpty() ? group : group + '.') + name;

			QCString paramStr;
			
			VCARD::ParamListIterator pit(p);
			
			for (; pit.current(); ++pit) {
				paramStr += pit.current()->asString();
			}

			QString valStr;
			
			if (v != 0) {
				valStr = v->asString();
			}
			else valStr = paramStr;
			
			ValueType type = XValue;
			
			switch (VCARD::EntityTypeToValueType(t)) {
			
				case VCARD::ValueSound:
					type = Sound;
					break;
				case VCARD::ValueAddress:
					type = Email;
					break;
				case VCARD::ValueTel:
				case VCARD::ValueTextList:
				case VCARD::ValueAgent:
				case VCARD::ValueOrg:
				case VCARD::ValueN:
				case VCARD::ValueText:
					type = Text;
					break;
				case VCARD::ValueUTC:
					type = UTCOffset;
					break;
				case VCARD::ValueURI:
					type = URL;
					break;
				case VCARD::ValueClass:
				case VCARD::ValueFloat:
					type = Double;
					break;
				case VCARD::ValueImage:
					type = Image;
					break;
				case VCARD::ValueDate:
					type = Date;
					type = Text;
					break;
				case VCARD::ValueTextBin:
				case VCARD::ValueUnknown:
					type = XValue;
					break;
			}
			
			Value val(valStr, type);
			Field f(n, val);
			e.add(f);
		}
		
		ab->add(e);
		
		++imported;
	}

	return imported;
}
}

#include <AdrParam.h>
#include <AdrValue.h>
#include <AgentParam.h>
#include <ContentLine.h>
#include <DateValue.h>
#include <EmailParam.h>
#include <Entity.h>
#include <ImgValue.h>
#include <LangValue.h>
#include <NValue.h>
#include <Param.h>
#include <PhoneNumberValue.h>
#include <SourceParam.h>
#include <TelParam.h>
#include <TextValue.h>
#include <URIValue.h>
#include <VCard.h>
#include <VCardEntity.h>
#include <iostream>
#include <qfile.h>
#include <qtextstream.h>

main(int argc, char * argv[])
{
	if (argc != 2) {
		cerr << "Usage: " << argv[0] << " <filename>" << endl;
		exit(1);
	}
	
	QFile f(argv[1]);
	
	QCString str;
	
	if (!f.open(IO_ReadOnly)) {
		cerr << "Couldn't open file \"" << argv[1] << endl;
		exit(1);
	}
	
	QTextStream t(&f);
	
	while (!t.eof())
		str += t.readLine() + '\n';
	
	// Iterate through all vCards in the file.

	VCARD::VCardEntity e(str);
	
	QListIterator<VCARD::VCard> it(e.cardList());
	
	for (; it.current(); ++it) {
		
		cerr << "****************** VCARD ********************" << endl;
		
		// Create a vcard using the string representation.
		VCARD::VCard & v (*it.current());

		if (v.has(VCARD::EntityEmail)) {
			cerr << "Email parameter found" << endl;
			
			QCString s = v.contentLine(VCARD::EntityEmail)->value()->asString();
			
			cerr << "Email value == " << s << endl;
		}
		
		if (v.has(VCARD::EntityNickname)) {
			cerr << "Nickname parameter found" << endl;
			
			cerr << "Nickname value == " <<
				v.contentLine(VCARD::EntityNickname)->value()->asString() <<
				endl;
		}
		
		if (v.has(VCARD::EntityRevision)) {
			
			cerr << "Revision parameter found" << endl;
			
			VCARD::DateValue * d =
				(VCARD::DateValue *)
				v.contentLine(VCARD::EntityRevision)->value();
			
			ASSERT(d != 0);
			
			cerr << "Revision date: " << endl;
			cerr << "Day   : " << d->day()		<< endl;
			cerr << "Month : " << d->month()	<< endl;
			cerr << "Year  : " << d->year()		<< endl;
			
			if (d->hasTime()) {
				cerr << "Revision date has a time component" << endl;
				cerr << "Revision time: " << endl;
				cerr << "Hour   : " << d->hour()	<< endl;
				cerr << "Minute : " << d->minute()	<< endl;
				cerr << "Second : " << d->second()	<< endl;

			}
			else cerr << "Revision date does NOT have a time component" << endl;
		}
		
		if (v.has(VCARD::EntityURL)) {
			cerr << "URL Parameter found" << endl;
			
			cerr << "URL Value == " <<
				v.contentLine(VCARD::EntityURL)->value()->asString() <<
				endl;
			
			VCARD::URIValue * urlVal =
				(VCARD::URIValue *)v.contentLine(VCARD::EntityURL)->value();

			ASSERT(urlVal != 0);
			
			cerr << "URL scheme == " <<
				urlVal->scheme() << endl;
			
			cerr << "URL scheme specific part == " <<
				urlVal->schemeSpecificPart() << endl;
		}
		
		if (v.has(VCARD::EntityN)) {
			cerr << "N Parameter found" << endl;
			
			VCARD::NValue * n =
				(VCARD::NValue *)(v.contentLine(VCARD::EntityN)->value());
			
			cerr << "Family name  == " << n->family()	<< endl;
			cerr << "Given  name  == " << n->given()	<< endl;
			cerr << "Middle name  == " << n->middle()	<< endl;
			cerr << "Prefix       == " << n->prefix()	<< endl;
			cerr << "Suffix       == " << n->suffix()	<< endl;
		}
		
		cerr << "***************** END VCARD ******************" << endl;
	}
}


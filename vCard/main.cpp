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
	
	f.open(IO_ReadOnly);
	
	QTextStream t(&f);
	
	while (!t.eof())
		str += t.readLine() + '\n';
	
	// Create a vcard using the string representation.

	VCARD::VCard v(str);

	if (v.has(VCARD::EntityEmail)) {
		cerr << "Email parameter found" << endl;
		
		QCString s = v.contentLine(VCARD::EntityEmail)->value()->asString();
		
		cerr << "Email value == " << s << endl;
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
	
	cerr << v.asString() << endl;
}


// XXX Automatically generated. DO NOT EDIT! XXX //

public:
LdifContent();
LdifContent(const LdifContent&);
LdifContent(const QCString&);
LdifContent & operator = (LdifContent&);
LdifContent & operator = (const QCString&);
bool operator ==(LdifContent&);
bool operator !=(LdifContent& x) {return !(*this==x);}
bool operator ==(const QCString& s) {LdifContent a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~LdifContent();
void _parse();
void _assemble();
const char * className() const { return "LdifContent"; }

// End of automatically generated code           //

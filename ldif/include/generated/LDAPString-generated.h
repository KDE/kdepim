// XXX Automatically generated. DO NOT EDIT! XXX //

public:
LDAPString();
LDAPString(const LDAPString&);
LDAPString(const QCString&);
LDAPString & operator = (LDAPString&);
LDAPString & operator = (const QCString&);
bool operator ==(LDAPString&);
bool operator !=(LDAPString& x) {return !(*this==x);}
bool operator ==(const QCString& s) {LDAPString a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~LDAPString();
void _parse();
void _assemble();
const char * className() const { return "LDAPString"; }

// End of automatically generated code           //

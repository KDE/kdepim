// XXX Automatically generated. DO NOT EDIT! XXX //

public:
Dn();
Dn(const Dn&);
Dn(const QCString&);
Dn & operator = (Dn&);
Dn & operator = (const QCString&);
bool operator ==(Dn&);
bool operator !=(Dn& x) {return !(*this==x);}
bool operator ==(const QCString& s) {Dn a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~Dn();
void _parse();
void _assemble();
const char * className() const { return "Dn"; }

// End of automatically generated code           //

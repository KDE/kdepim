// XXX Automatically generated. DO NOT EDIT! XXX //

public:
LdifAttrValRec();
LdifAttrValRec(const LdifAttrValRec&);
LdifAttrValRec(const QCString&);
LdifAttrValRec & operator = (LdifAttrValRec&);
LdifAttrValRec & operator = (const QCString&);
bool operator ==(LdifAttrValRec&);
bool operator !=(LdifAttrValRec& x) {return !(*this==x);}
bool operator ==(const QCString& s) {LdifAttrValRec a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~LdifAttrValRec();
void _parse();
void _assemble();
const char * className() const { return "LdifAttrValRec"; }

// End of automatically generated code           //

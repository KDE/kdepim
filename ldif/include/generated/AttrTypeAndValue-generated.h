// XXX Automatically generated. DO NOT EDIT! XXX //

public:
AttrTypeAndValue();
AttrTypeAndValue(const AttrTypeAndValue&);
AttrTypeAndValue(const QCString&);
AttrTypeAndValue & operator = (AttrTypeAndValue&);
AttrTypeAndValue & operator = (const QCString&);
bool operator ==(AttrTypeAndValue&);
bool operator !=(AttrTypeAndValue& x) {return !(*this==x);}
bool operator ==(const QCString& s) {AttrTypeAndValue a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~AttrTypeAndValue();
void _parse();
void _assemble();
const char * className() const { return "AttrTypeAndValue"; }

// End of automatically generated code           //

// XXX Automatically generated. DO NOT EDIT! XXX //

public:
AttrValSpec();
AttrValSpec(const AttrValSpec&);
AttrValSpec(const QCString&);
AttrValSpec & operator = (AttrValSpec&);
AttrValSpec & operator = (const QCString&);
bool operator ==(AttrValSpec&);
bool operator !=(AttrValSpec& x) {return !(*this==x);}
bool operator ==(const QCString& s) {AttrValSpec a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~AttrValSpec();
void _parse();
void _assemble();
const char * className() const { return "AttrValSpec"; }

// End of automatically generated code           //

// XXX Automatically generated. DO NOT EDIT! XXX //

public:
ValueSpec();
ValueSpec(const ValueSpec&);
ValueSpec(const QCString&);
ValueSpec & operator = (ValueSpec&);
ValueSpec & operator = (const QCString&);
bool operator ==(ValueSpec&);
bool operator !=(ValueSpec& x) {return !(*this==x);}
bool operator ==(const QCString& s) {ValueSpec a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~ValueSpec();
void _parse();
void _assemble();
const char * className() const { return "ValueSpec"; }

// End of automatically generated code           //

// XXX Automatically generated. DO NOT EDIT! XXX //

public:
DnSpec();
DnSpec(const DnSpec&);
DnSpec(const QCString&);
DnSpec & operator = (DnSpec&);
DnSpec & operator = (const QCString&);
bool operator ==(DnSpec&);
bool operator !=(DnSpec& x) {return !(*this==x);}
bool operator ==(const QCString& s) {DnSpec a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~DnSpec();
void _parse();
void _assemble();
const char * className() const { return "DnSpec"; }

// End of automatically generated code           //

// XXX Automatically generated. DO NOT EDIT! XXX //

public:
VersionSpec();
VersionSpec(const VersionSpec&);
VersionSpec(const QCString&);
VersionSpec & operator = (VersionSpec&);
VersionSpec & operator = (const QCString&);
bool operator ==(VersionSpec&);
bool operator !=(VersionSpec& x) {return !(*this==x);}
bool operator ==(const QCString& s) {VersionSpec a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~VersionSpec();
void _parse();
void _assemble();
const char * className() const { return "VersionSpec"; }

// End of automatically generated code           //

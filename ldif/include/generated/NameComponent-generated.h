// XXX Automatically generated. DO NOT EDIT! XXX //

public:
NameComponent();
NameComponent(const NameComponent&);
NameComponent(const QCString&);
NameComponent & operator = (NameComponent&);
NameComponent & operator = (const QCString&);
bool operator ==(NameComponent&);
bool operator !=(NameComponent& x) {return !(*this==x);}
bool operator ==(const QCString& s) {NameComponent a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~NameComponent();
void _parse();
void _assemble();
const char * className() const { return "NameComponent"; }

// End of automatically generated code           //

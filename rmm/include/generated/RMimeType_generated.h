// XXX Automatically generated. DO NOT EDIT! XXX //

public:
RMimeType();
RMimeType(const RMimeType &);
RMimeType(const QCString &);
RMimeType & operator = (const RMimeType &);
RMimeType & operator = (const QCString &);
bool operator == (RMimeType &);
bool operator != (RMimeType & x) { return !(*this == x); }
bool operator == (const QCString & s) { RMimeType a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RMimeType();
void createDefault();

const char * className() const { return "RMimeType"; }

protected:
void _parse();
void _assemble();

// End of automatically generated code           //

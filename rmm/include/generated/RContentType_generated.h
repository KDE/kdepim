// XXX Automatically generated. DO NOT EDIT! XXX //

public:
RContentType();
RContentType(const RContentType &);
RContentType(const QCString &);
RContentType & operator = (const RContentType &);
RContentType & operator = (const QCString &);
bool operator == (RContentType &);
bool operator != (RContentType & x) { return !(*this == x); }
bool operator == (const QCString & s) { RContentType a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RContentType();
void createDefault();

const char * className() const { return "RContentType"; }

protected:
void _parse();
void _assemble();

// End of automatically generated code           //

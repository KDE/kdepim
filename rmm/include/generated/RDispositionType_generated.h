// XXX Automatically generated. DO NOT EDIT! XXX //

public:
RDispositionType();
RDispositionType(const RDispositionType &);
RDispositionType(const QCString &);
RDispositionType & operator = (const RDispositionType &);
RDispositionType & operator = (const QCString &);
bool operator == (RDispositionType &);
bool operator != (RDispositionType & x) { return !(*this == x); }
bool operator == (const QCString & s) { RDispositionType a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RDispositionType();
void createDefault();

const char * className() const { return "RDispositionType"; }

protected:
void _parse();
void _assemble();

// End of automatically generated code           //

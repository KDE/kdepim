// XXX Automatically generated. DO NOT EDIT! XXX //

public:
RDispositionType();
RDispositionType(const RDispositionType &);
RDispositionType(const QCString &);
RDispositionType & operator = (const RDispositionType &);
RDispositionType & operator = (const QCString &);
friend QDataStream & operator >> (QDataStream & s, RDispositionType &);
friend QDataStream & operator << (QDataStream & s, RDispositionType &);
bool operator == (RDispositionType &);
bool operator != (RDispositionType & x) { return !(*this == x); }
bool operator == (const QCString & s) { RDispositionType a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RDispositionType();
bool isNull() { parse(); return strRep_.isEmpty(); }
bool operator ! () { return isNull(); }
void createDefault();

const char * className() const { return "RDispositionType"; }

protected:
void _parse();
void _assemble();

// End of automatically generated code           //

// XXX Automatically generated. DO NOT EDIT! XXX //

public:
RHeader();
RHeader(const RHeader &);
RHeader(const QCString &);
RHeader & operator = (const RHeader &);
RHeader & operator = (const QCString &);
friend QDataStream & operator >> (QDataStream & s, RHeader &);
friend QDataStream & operator << (QDataStream & s, RHeader &);
bool operator == (RHeader &);
bool operator != (RHeader & x) { return !(*this == x); }
bool operator == (const QCString & s) { RHeader a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RHeader();
bool isNull() { parse(); return strRep_.isEmpty(); }
bool operator ! () { return isNull(); }
void createDefault();

const char * className() const { return "RHeader"; }

protected:
void _parse();
void _assemble();

// End of automatically generated code           //

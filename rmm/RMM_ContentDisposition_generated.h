// XXX Automatically generated. DO NOT EDIT! XXX //

public:
RContentDisposition();
RContentDisposition(const RContentDisposition &);
RContentDisposition(const QCString &);
RContentDisposition & operator = (const RContentDisposition &);
RContentDisposition & operator = (const QCString &);
friend QDataStream & operator >> (QDataStream & s, RContentDisposition &);
friend QDataStream & operator << (QDataStream & s, RContentDisposition &);
bool operator == (RContentDisposition &);
bool operator != (RContentDisposition & x) { return !(*this == x); }
bool operator == (const QCString & s) { RContentDisposition a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RContentDisposition();
bool isNull() { parse(); return strRep_.isEmpty(); }
bool operator ! () { return isNull(); }
void createDefault();

const char * className() const { return "RContentDisposition"; }

protected:
void _parse();
void _assemble();

// End of automatically generated code           //

// XXX Automatically generated. DO NOT EDIT! XXX //

public:
RAddress();
RAddress(const RAddress &);
RAddress(const QCString &);
RAddress & operator = (const RAddress &);
RAddress & operator = (const QCString &);
friend QDataStream & operator >> (QDataStream & s, RAddress &);
friend QDataStream & operator << (QDataStream & s, RAddress &);
bool operator == (RAddress &);
bool operator != (RAddress & x) { return !(*this == x); }
bool operator == (const QCString & s) { RAddress a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RAddress();
bool isNull() { parse(); return strRep_.isEmpty(); }
bool operator ! () { return isNull(); }
void createDefault();

const char * className() const { return "RAddress"; }

protected:
void _parse();
void _assemble();

// End of automatically generated code           //

// XXX Automatically generated. DO NOT EDIT! XXX //

public:
RDateTime();
RDateTime(const RDateTime &);
RDateTime(const QCString &);
RDateTime & operator = (const RDateTime &);
RDateTime & operator = (const QCString &);
friend QDataStream & operator >> (QDataStream & s, RDateTime &);
friend QDataStream & operator << (QDataStream & s, RDateTime &);
bool operator == (RDateTime &);
bool operator != (RDateTime & x) { return !(*this == x); }
bool operator == (const QCString & s) { RDateTime a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RDateTime();
bool isNull() { parse(); return strRep_.isEmpty(); }
bool operator ! () { return isNull(); }
void createDefault();

const char * className() const { return "RDateTime"; }

protected:
void _parse();
void _assemble();

// End of automatically generated code           //

// XXX Automatically generated. DO NOT EDIT! XXX //

public:
RCte();
RCte(const RCte &);
RCte(const QCString &);
RCte & operator = (const RCte &);
RCte & operator = (const QCString &);
friend QDataStream & operator >> (QDataStream & s, RCte &);
friend QDataStream & operator << (QDataStream & s, RCte &);
bool operator == (RCte &);
bool operator != (RCte & x) { return !(*this == x); }
bool operator == (const QCString & s) { RCte a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RCte();
bool isNull() { parse(); return strRep_.isEmpty(); }
bool operator ! () { return isNull(); }
void createDefault();

const char * className() const { return "RCte"; }

protected:
void _parse();
void _assemble();

// End of automatically generated code           //

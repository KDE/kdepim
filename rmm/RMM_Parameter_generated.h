// XXX Automatically generated. DO NOT EDIT! XXX //

public:
RParameter();
RParameter(const RParameter &);
RParameter(const QCString &);
RParameter & operator = (const RParameter &);
RParameter & operator = (const QCString &);
friend QDataStream & operator >> (QDataStream & s, RParameter &);
friend QDataStream & operator << (QDataStream & s, RParameter &);
bool operator == (RParameter &);
bool operator != (RParameter & x) { return !(*this == x); }
bool operator == (const QCString & s) { RParameter a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RParameter();
bool isNull() { parse(); return strRep_.isEmpty(); }
bool operator ! () { return isNull(); }
void createDefault();

const char * className() const { return "RParameter"; }

protected:
void _parse();
void _assemble();

// End of automatically generated code           //

// XXX Automatically generated. DO NOT EDIT! XXX //

public:
RMessage();
RMessage(const RMessage &);
RMessage(const QCString &);
RMessage & operator = (const RMessage &);
RMessage & operator = (const QCString &);
friend QDataStream & operator >> (QDataStream & s, RMessage &);
friend QDataStream & operator << (QDataStream & s, RMessage &);
bool operator == (RMessage &);
bool operator != (RMessage & x) { return !(*this == x); }
bool operator == (const QCString & s) { RMessage a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RMessage();
bool isNull() { parse(); return strRep_.isEmpty(); }
bool operator ! () { return isNull(); }
void createDefault();

const char * className() const { return "RMessage"; }

protected:
void _parse();
void _assemble();

// End of automatically generated code           //

// XXX Automatically generated. DO NOT EDIT! XXX //

public:
RParameterList();
RParameterList(const RParameterList &);
RParameterList(const QCString &);
RParameterList & operator = (const RParameterList &);
RParameterList & operator = (const QCString &);
friend QDataStream & operator >> (QDataStream & s, RParameterList &);
friend QDataStream & operator << (QDataStream & s, RParameterList &);
bool operator == (RParameterList &);
bool operator != (RParameterList & x) { return !(*this == x); }
bool operator == (const QCString & s) { RParameterList a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RParameterList();
bool isNull() { parse(); return strRep_.isEmpty(); }
bool operator ! () { return isNull(); }
void createDefault();

const char * className() const { return "RParameterList"; }

protected:
void _parse();
void _assemble();

// End of automatically generated code           //

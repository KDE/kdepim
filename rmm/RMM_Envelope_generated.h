// XXX Automatically generated. DO NOT EDIT! XXX //

public:
REnvelope();
REnvelope(const REnvelope &);
REnvelope(const QCString &);
REnvelope & operator = (const REnvelope &);
REnvelope & operator = (const QCString &);
friend QDataStream & operator >> (QDataStream & s, REnvelope &);
friend QDataStream & operator << (QDataStream & s, REnvelope &);
bool operator == (REnvelope &);
bool operator != (REnvelope & x) { return !(*this == x); }
bool operator == (const QCString & s) { REnvelope a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~REnvelope();
bool isNull() { parse(); return strRep_.isEmpty(); }
bool operator ! () { return isNull(); }
void createDefault();

const char * className() const { return "REnvelope"; }

protected:
void _parse();
void _assemble();

// End of automatically generated code           //

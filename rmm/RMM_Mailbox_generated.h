// XXX Automatically generated. DO NOT EDIT! XXX //

public:
RMailbox();
RMailbox(const RMailbox &);
RMailbox(const QCString &);
RMailbox & operator = (const RMailbox &);
RMailbox & operator = (const QCString &);
friend QDataStream & operator >> (QDataStream & s, RMailbox &);
friend QDataStream & operator << (QDataStream & s, RMailbox &);
bool operator == (RMailbox &);
bool operator != (RMailbox & x) { return !(*this == x); }
bool operator == (const QCString & s) { RMailbox a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RMailbox();
bool isNull() { parse(); return strRep_.isEmpty(); }
bool operator ! () { return isNull(); }
void createDefault();

const char * className() const { return "RMailbox"; }

protected:
void _parse();
void _assemble();

// End of automatically generated code           //

// XXX Automatically generated. DO NOT EDIT! XXX //

public:
RCte();
RCte(const RCte &);
RCte(const QCString &);
RCte & operator = (const RCte &);
RCte & operator = (const QCString &);
bool operator == (RCte &);
bool operator != (RCte & x) { return !(*this == x); }
bool operator == (const QCString & s) { RCte a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RCte();
void createDefault();

const char * className() const { return "RCte"; }

protected:
void _parse();
void _assemble();

// End of automatically generated code           //

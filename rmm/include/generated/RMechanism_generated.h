// XXX Automatically generated. DO NOT EDIT! XXX //

public:
RMechanism();
RMechanism(const RMechanism &);
RMechanism(const QCString &);
RMechanism & operator = (const RMechanism &);
RMechanism & operator = (const QCString &);
bool operator == (RMechanism &);
bool operator != (RMechanism & x) { return !(*this == x); }
bool operator == (const QCString & s) { RMechanism a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RMechanism();
void createDefault();

const char * className() const { return "RMechanism"; }

protected:
void _parse();
void _assemble();

// End of automatically generated code           //

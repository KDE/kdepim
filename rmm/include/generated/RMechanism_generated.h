// XXX Automatically generated. DO NOT EDIT! XXX //

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
void _parse();
void _assemble();
void parse() 			{ if (!parsed_) _parse(); parsed_ = true; assembled_ = false; }

void assemble() 			{ parse() ; if (!assembled_) _assemble(); assembled_ = true;}

void createDefault();

const char * className() const { return "RMechanism"; }

// End of automatically generated code           //

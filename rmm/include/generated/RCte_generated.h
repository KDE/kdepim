// XXX Automatically generated. DO NOT EDIT! XXX //

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
void _parse();
void _assemble();
void parse() 			{ if (!parsed_) _parse(); parsed_ = true; assembled_ = false; }

void assemble() 			{ parse() ; if (!assembled_) _assemble(); assembled_ = true;}

void createDefault();

const char * className() const { return "RCte"; }

// End of automatically generated code           //

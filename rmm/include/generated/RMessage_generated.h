// XXX Automatically generated. DO NOT EDIT! XXX //

RMessage();
RMessage(const RMessage &);
RMessage(const QCString &);
RMessage & operator = (const RMessage &);
RMessage & operator = (const QCString &);
bool operator == (RMessage &);
bool operator != (RMessage & x) { return !(*this == x); }
bool operator == (const QCString & s) { RMessage a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RMessage();
void _parse();
void _assemble();
void parse() 			{ if (!parsed_) _parse(); parsed_ = true; assembled_ = false; }

void assemble() 			{ parse() ; if (!assembled_) _assemble(); assembled_ = true;}

void createDefault();

const char * className() const { return "RMessage"; }

// End of automatically generated code           //

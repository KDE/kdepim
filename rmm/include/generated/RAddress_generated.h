// XXX Automatically generated. DO NOT EDIT! XXX //

RAddress();
RAddress(const RAddress &);
RAddress(const QCString &);
RAddress & operator = (const RAddress &);
RAddress & operator = (const QCString &);
bool operator == (RAddress &);
bool operator != (RAddress & x) { return !(*this == x); }
bool operator == (const QCString & s) { RAddress a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RAddress();
void _parse();
void _assemble();
void parse() 			{ if (!parsed_) _parse(); parsed_ = true; assembled_ = false; }

void assemble() 			{ parse() ; if (!assembled_) _assemble(); assembled_ = true;}

void createDefault();

const char * className() const { return "RAddress"; }

// End of automatically generated code           //

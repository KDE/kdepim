// XXX Automatically generated. DO NOT EDIT! XXX //

RHeader();
RHeader(const RHeader &);
RHeader(const QCString &);
RHeader & operator = (const RHeader &);
RHeader & operator = (const QCString &);
bool operator == (RHeader &);
bool operator != (RHeader & x) { return !(*this == x); }
bool operator == (const QCString & s) { RHeader a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RHeader();
void _parse();
void _assemble();
void parse() 			{ if (!parsed_) _parse(); parsed_ = true; assembled_ = false; }

void assemble() 			{ parse() ; if (!assembled_) _assemble(); assembled_ = true;}

void createDefault();

const char * className() const { return "RHeader"; }

// End of automatically generated code           //

// XXX Automatically generated. DO NOT EDIT! XXX //

RDateTime();
RDateTime(const RDateTime &);
RDateTime(const QCString &);
RDateTime & operator = (const RDateTime &);
RDateTime & operator = (const QCString &);
bool operator == (RDateTime &);
bool operator != (RDateTime & x) { return !(*this == x); }
bool operator == (const QCString & s) { RDateTime a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RDateTime();
void _parse();
void _assemble();
void parse() 			{ if (!parsed_) _parse(); parsed_ = true; assembled_ = false; }

void assemble() 			{ parse() ; if (!assembled_) _assemble(); assembled_ = true;}

void createDefault();

const char * className() const { return "RDateTime"; }

// End of automatically generated code           //

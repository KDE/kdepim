// XXX Automatically generated. DO NOT EDIT! XXX //

RParameter();
RParameter(const RParameter &);
RParameter(const QCString &);
RParameter & operator = (const RParameter &);
RParameter & operator = (const QCString &);
bool operator == (RParameter &);
bool operator != (RParameter & x) { return !(*this == x); }
bool operator == (const QCString & s) { RParameter a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RParameter();
void _parse();
void _assemble();
void parse() 			{ if (!parsed_) _parse(); parsed_ = true; assembled_ = false; }

void assemble() 			{ parse() ; if (!assembled_) _assemble(); assembled_ = true;}

void createDefault();

const char * className() const { return "RParameter"; }

// End of automatically generated code           //

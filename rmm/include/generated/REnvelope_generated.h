// XXX Automatically generated. DO NOT EDIT! XXX //

REnvelope();
REnvelope(const REnvelope &);
REnvelope(const QCString &);
REnvelope & operator = (const REnvelope &);
REnvelope & operator = (const QCString &);
bool operator == (REnvelope &);
bool operator != (REnvelope & x) { return !(*this == x); }
bool operator == (const QCString & s) { REnvelope a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~REnvelope();
void _parse();
void _assemble();
void parse() 			{ if (!parsed_) _parse(); parsed_ = true; assembled_ = false; }

void assemble() 			{ parse() ; if (!assembled_) _assemble(); assembled_ = true;}

void createDefault();

const char * className() const { return "REnvelope"; }

// End of automatically generated code           //

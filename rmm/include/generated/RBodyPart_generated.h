// XXX Automatically generated. DO NOT EDIT! XXX //

RBodyPart();
RBodyPart(const RBodyPart &);
RBodyPart(const QCString &);
RBodyPart & operator = (const RBodyPart &);
RBodyPart & operator = (const QCString &);
bool operator == (RBodyPart &);
bool operator != (RBodyPart & x) { return !(*this == x); }
bool operator == (const QCString & s) { RBodyPart a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RBodyPart();
void _parse();
void _assemble();
void parse() 			{ if (!parsed_) _parse(); parsed_ = true; assembled_ = false; }

void assemble() 			{ parse() ; if (!assembled_) _assemble(); assembled_ = true;}

void createDefault();

const char * className() const { return "RBodyPart"; }

// End of automatically generated code           //

// XXX Automatically generated. DO NOT EDIT! XXX //

RContentType();
RContentType(const RContentType &);
RContentType(const QCString &);
RContentType & operator = (const RContentType &);
RContentType & operator = (const QCString &);
bool operator == (RContentType &);
bool operator != (RContentType & x) { return !(*this == x); }
bool operator == (const QCString & s) { RContentType a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RContentType();
void _parse();
void _assemble();
void parse() 			{ if (!parsed_) _parse(); parsed_ = true; assembled_ = false; }

void assemble() 			{ parse() ; if (!assembled_) _assemble(); assembled_ = true;}

void createDefault();

const char * className() const { return "RContentType"; }

// End of automatically generated code           //

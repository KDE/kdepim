// XXX Automatically generated. DO NOT EDIT! XXX //

RMimeType();
RMimeType(const RMimeType &);
RMimeType(const QCString &);
RMimeType & operator = (const RMimeType &);
RMimeType & operator = (const QCString &);
bool operator == (RMimeType &);
bool operator != (RMimeType & x) { return !(*this == x); }
bool operator == (const QCString & s) { RMimeType a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RMimeType();
void _parse();
void _assemble();
void parse() 			{ if (!parsed_) _parse(); parsed_ = true; assembled_ = false; }

void assemble() 			{ parse() ; if (!assembled_) _assemble(); assembled_ = true;}

void createDefault();

const char * className() const { return "RMimeType"; }

// End of automatically generated code           //

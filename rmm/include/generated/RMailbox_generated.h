// XXX Automatically generated. DO NOT EDIT! XXX //

RMailbox();
RMailbox(const RMailbox &);
RMailbox(const QCString &);
RMailbox & operator = (const RMailbox &);
RMailbox & operator = (const QCString &);
bool operator == (RMailbox &);
bool operator != (RMailbox & x) { return !(*this == x); }
bool operator == (const QCString & s) { RMailbox a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RMailbox();
void _parse();
void _assemble();
void parse() 			{ if (!parsed_) _parse(); parsed_ = true; assembled_ = false; }

void assemble() 			{ parse() ; if (!assembled_) _assemble(); assembled_ = true;}

void createDefault();

const char * className() const { return "RMailbox"; }

// End of automatically generated code           //

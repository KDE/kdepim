// XXX Automatically generated. DO NOT EDIT! XXX //

RText();
RText(const RText &);
RText(const QCString &);
RText & operator = (const RText &);
RText & operator = (const QCString &);
bool operator == (RText &);
bool operator != (RText & x) { return !(*this == x); }
bool operator == (const QCString & s) { RText a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RText();
void _parse();
void _assemble();
void parse() 			{ if (!parsed_) _parse(); parsed_ = true; assembled_ = false; }

void assemble() 			{ parse() ; if (!assembled_) _assemble(); assembled_ = true;}

void createDefault();

const char * className() const { return "RText"; }

// End of automatically generated code           //

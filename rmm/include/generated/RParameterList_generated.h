// XXX Automatically generated. DO NOT EDIT! XXX //

RParameterList();
RParameterList(const RParameterList &);
RParameterList(const QCString &);
RParameterList & operator = (const RParameterList &);
RParameterList & operator = (const QCString &);
bool operator == (RParameterList &);
bool operator != (RParameterList & x) { return !(*this == x); }
bool operator == (const QCString & s) { RParameterList a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RParameterList();
void _parse();
void _assemble();
void parse() 			{ if (!parsed_) _parse(); parsed_ = true; assembled_ = false; }

void assemble() 			{ parse() ; if (!assembled_) _assemble(); assembled_ = true;}

void createDefault();

const char * className() const { return "RParameterList"; }

// End of automatically generated code           //

// XXX Automatically generated. DO NOT EDIT! XXX //

RAddressList();
RAddressList(const RAddressList &);
RAddressList(const QCString &);
RAddressList & operator = (const RAddressList &);
RAddressList & operator = (const QCString &);
bool operator == (RAddressList &);
bool operator != (RAddressList & x) { return !(*this == x); }
bool operator == (const QCString & s) { RAddressList a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RAddressList();
void _parse();
void _assemble();
void parse() 			{ if (!parsed_) _parse(); parsed_ = true; assembled_ = false; }

void assemble() 			{ parse() ; if (!assembled_) _assemble(); assembled_ = true;}

void createDefault();

const char * className() const { return "RAddressList"; }

// End of automatically generated code           //

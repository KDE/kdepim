// XXX Automatically generated. DO NOT EDIT! XXX //

RMessageID();
RMessageID(const RMessageID &);
RMessageID(const QCString &);
RMessageID & operator = (const RMessageID &);
RMessageID & operator = (const QCString &);
bool operator == (RMessageID &);
bool operator != (RMessageID & x) { return !(*this == x); }
bool operator == (const QCString & s) { RMessageID a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RMessageID();
void _parse();
void _assemble();
void parse() 			{ if (!parsed_) _parse(); parsed_ = true; assembled_ = false; }

void assemble() 			{ parse() ; if (!assembled_) _assemble(); assembled_ = true;}

void createDefault();

const char * className() const { return "RMessageID"; }

// End of automatically generated code           //

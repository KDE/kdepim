// XXX Automatically generated. DO NOT EDIT! XXX //

RGroup();
RGroup(const RGroup &);
RGroup(const QCString &);
RGroup & operator = (const RGroup &);
RGroup & operator = (const QCString &);
bool operator == (RGroup &);
bool operator != (RGroup & x) { return !(*this == x); }
bool operator == (const QCString & s) { RGroup a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RGroup();
void _parse();
void _assemble();
void parse() 			{ if (!parsed_) _parse(); parsed_ = true; assembled_ = false; }

void assemble() 			{ parse() ; if (!assembled_) _assemble(); assembled_ = true;}

void createDefault();

const char * className() const { return "RGroup"; }

// End of automatically generated code           //

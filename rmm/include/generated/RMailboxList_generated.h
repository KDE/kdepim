// XXX Automatically generated. DO NOT EDIT! XXX //

RMailboxList();
RMailboxList(const RMailboxList &);
RMailboxList(const QCString &);
RMailboxList & operator = (const RMailboxList &);
RMailboxList & operator = (const QCString &);
bool operator == (RMailboxList &);
bool operator != (RMailboxList & x) { return !(*this == x); }
bool operator == (const QCString & s) { RMailboxList a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RMailboxList();
void _parse();
void _assemble();
void parse() 			{ if (!parsed_) _parse(); parsed_ = true; assembled_ = false; }

void assemble() 			{ parse() ; if (!assembled_) _assemble(); assembled_ = true;}

void createDefault();

const char * className() const { return "RMailboxList"; }

// End of automatically generated code           //

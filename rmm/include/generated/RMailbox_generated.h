// XXX Automatically generated. DO NOT EDIT! XXX //

public:
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
void createDefault();

const char * className() const { return "RMailbox"; }

protected:
void _parse();
void _assemble();

// End of automatically generated code           //

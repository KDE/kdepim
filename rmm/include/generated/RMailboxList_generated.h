// XXX Automatically generated. DO NOT EDIT! XXX //

public:
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
void createDefault();

const char * className() const { return "RMailboxList"; }

protected:
void _parse();
void _assemble();

// End of automatically generated code           //

// XXX Automatically generated. DO NOT EDIT! XXX //

public:
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
void createDefault();

const char * className() const { return "RAddressList"; }

protected:
void _parse();
void _assemble();

// End of automatically generated code           //

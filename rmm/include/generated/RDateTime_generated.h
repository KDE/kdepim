// XXX Automatically generated. DO NOT EDIT! XXX //

public:
RDateTime();
RDateTime(const RDateTime &);
RDateTime(const QCString &);
RDateTime & operator = (const RDateTime &);
RDateTime & operator = (const QCString &);
bool operator == (RDateTime &);
bool operator != (RDateTime & x) { return !(*this == x); }
bool operator == (const QCString & s) { RDateTime a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RDateTime();
void createDefault();

const char * className() const { return "RDateTime"; }

protected:
void _parse();
void _assemble();

// End of automatically generated code           //

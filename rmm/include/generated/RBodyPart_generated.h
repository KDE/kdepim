// XXX Automatically generated. DO NOT EDIT! XXX //

public:
RBodyPart();
RBodyPart(const RBodyPart &);
RBodyPart(const QCString &);
RBodyPart & operator = (const RBodyPart &);
RBodyPart & operator = (const QCString &);
bool operator == (RBodyPart &);
bool operator != (RBodyPart & x) { return !(*this == x); }
bool operator == (const QCString & s) { RBodyPart a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RBodyPart();
void createDefault();

const char * className() const { return "RBodyPart"; }

protected:
void _parse();
void _assemble();

// End of automatically generated code           //

// XXX Automatically generated. DO NOT EDIT! XXX //

public:
RText();
RText(const RText &);
RText(const QCString &);
RText & operator = (const RText &);
RText & operator = (const QCString &);
bool operator == (RText &);
bool operator != (RText & x) { return !(*this == x); }
bool operator == (const QCString & s) { RText a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RText();
void createDefault();

const char * className() const { return "RText"; }

protected:
void _parse();
void _assemble();

// End of automatically generated code           //

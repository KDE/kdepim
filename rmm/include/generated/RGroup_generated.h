// XXX Automatically generated. DO NOT EDIT! XXX //

public:
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
void createDefault();

const char * className() const { return "RGroup"; }

protected:
void _parse();
void _assemble();

// End of automatically generated code           //

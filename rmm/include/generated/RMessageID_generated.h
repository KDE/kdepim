// XXX Automatically generated. DO NOT EDIT! XXX //

public:
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
void createDefault();

const char * className() const { return "RMessageID"; }

protected:
void _parse();
void _assemble();

// End of automatically generated code           //

// XXX Automatically generated. DO NOT EDIT! XXX //

public:
RHeaderBody();
RHeaderBody(const RHeaderBody &);
RHeaderBody(const QCString &);
RHeaderBody & operator = (const RHeaderBody &);
RHeaderBody & operator = (const QCString &);
bool operator == (RHeaderBody &);
bool operator != (RHeaderBody & x) { return !(*this == x); }
bool operator == (const QCString & s) { RHeaderBody a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RHeaderBody();
virtual void createDefault();

virtual const char * className() const { return "RHeaderBody"; }

protected:
virtual void _parse();
virtual void _assemble();

// End of automatically generated code           //

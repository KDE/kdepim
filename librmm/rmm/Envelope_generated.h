// XXX Automatically generated. DO NOT EDIT! XXX //

public:
Envelope();
Envelope(const Envelope &);
Envelope(const QCString &);
Envelope & operator = (const Envelope &);
Envelope & operator = (const QCString &);
friend QDataStream & operator >> (QDataStream & s, Envelope &);
friend QDataStream & operator << (QDataStream & s, Envelope &);
bool operator == (Envelope &);
bool operator != (Envelope & x) { return !(*this == x); }
bool operator == (const QCString & s) { Envelope a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~Envelope();
virtual bool isNull() { parse(); return strRep_.isEmpty(); }
virtual bool operator ! () { return isNull(); }
virtual void createDefault();

virtual const char * className() const { return "Envelope"; }

protected:
virtual void _parse();
virtual void _assemble();

// End of automatically generated code           //

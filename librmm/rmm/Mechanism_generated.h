// XXX Automatically generated. DO NOT EDIT! XXX //

public:
Mechanism();
Mechanism(const Mechanism &);
Mechanism(const QCString &);
Mechanism & operator = (const Mechanism &);
Mechanism & operator = (const QCString &);
friend QDataStream & operator >> (QDataStream & s, Mechanism &);
friend QDataStream & operator << (QDataStream & s, Mechanism &);
bool operator == (Mechanism &);
bool operator != (Mechanism & x) { return !(*this == x); }
bool operator == (const QCString & s) { Mechanism a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~Mechanism();
virtual bool isNull() { parse(); return strRep_.isEmpty(); }
virtual bool operator ! () { return isNull(); }
virtual void createDefault();

virtual const char * className() const { return "Mechanism"; }

protected:
virtual void _parse();
virtual void _assemble();

// End of automatically generated code           //

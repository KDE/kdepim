// XXX Automatically generated. DO NOT EDIT! XXX //

public:
Parameter();
Parameter(const Parameter &);
Parameter(const QCString &);
Parameter & operator = (const Parameter &);
Parameter & operator = (const QCString &);
friend QDataStream & operator >> (QDataStream & s, Parameter &);
friend QDataStream & operator << (QDataStream & s, Parameter &);
bool operator == (Parameter &);
bool operator != (Parameter & x) { return !(*this == x); }
bool operator == (const QCString & s) { Parameter a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~Parameter();
virtual bool isNull() { parse(); return strRep_.isEmpty(); }
virtual bool operator ! () { return isNull(); }
virtual void createDefault();

virtual const char * className() const { return "Parameter"; }

protected:
virtual void _parse();
virtual void _assemble();

// End of automatically generated code           //

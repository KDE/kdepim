// XXX Automatically generated. DO NOT EDIT! XXX //

public:
ParameterList();
ParameterList(const ParameterList &);
ParameterList(const QCString &);
ParameterList & operator = (const ParameterList &);
ParameterList & operator = (const QCString &);
friend QDataStream & operator >> (QDataStream & s, ParameterList &);
friend QDataStream & operator << (QDataStream & s, ParameterList &);
bool operator == (ParameterList &);
bool operator != (ParameterList & x) { return !(*this == x); }
bool operator == (const QCString & s) { ParameterList a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~ParameterList();
virtual bool isNull() { parse(); return strRep_.isEmpty(); }
virtual bool operator ! () { return isNull(); }
virtual void createDefault();

virtual const char * className() const { return "ParameterList"; }

protected:
virtual void _parse();
virtual void _assemble();

// End of automatically generated code           //

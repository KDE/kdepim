// XXX Automatically generated. DO NOT EDIT! XXX //

public:
DateTime();
DateTime(const DateTime &);
DateTime(const QCString &);
DateTime & operator = (const DateTime &);
DateTime & operator = (const QCString &);
friend QDataStream & operator >> (QDataStream & s, DateTime &);
friend QDataStream & operator << (QDataStream & s, DateTime &);
bool operator == (DateTime &);
bool operator != (DateTime & x) { return !(*this == x); }
bool operator == (const QCString & s) { DateTime a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~DateTime();
virtual bool isNull() { parse(); return strRep_.isEmpty(); }
virtual bool operator ! () { return isNull(); }
virtual void createDefault();

virtual const char * className() const { return "DateTime"; }

protected:
virtual void _parse();
virtual void _assemble();

// End of automatically generated code           //

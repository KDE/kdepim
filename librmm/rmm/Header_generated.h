// XXX Automatically generated. DO NOT EDIT! XXX //

public:
Header();
Header(const Header &);
Header(const QCString &);
Header & operator = (const Header &);
Header & operator = (const QCString &);
friend QDataStream & operator >> (QDataStream & s, Header &);
friend QDataStream & operator << (QDataStream & s, Header &);
bool operator == (Header &);
bool operator != (Header & x) { return !(*this == x); }
bool operator == (const QCString & s) { Header a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~Header();
virtual bool isNull() { parse(); return strRep_.isEmpty(); }
virtual bool operator ! () { return isNull(); }
virtual void createDefault();

virtual const char * className() const { return "Header"; }

protected:
virtual void _parse();
virtual void _assemble();

// End of automatically generated code           //

// XXX Automatically generated. DO NOT EDIT! XXX //

public:
Address();
Address(const Address &);
Address(const QCString &);
Address & operator = (const Address &);
Address & operator = (const QCString &);
friend QDataStream & operator >> (QDataStream & s, Address &);
friend QDataStream & operator << (QDataStream & s, Address &);
bool operator == (Address &);
bool operator != (Address & x) { return !(*this == x); }
bool operator == (const QCString & s) { Address a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~Address();
virtual bool isNull() { parse(); return strRep_.isEmpty(); }
virtual bool operator ! () { return isNull(); }
virtual void createDefault();

virtual const char * className() const { return "Address"; }

protected:
virtual void _parse();
virtual void _assemble();

// End of automatically generated code           //

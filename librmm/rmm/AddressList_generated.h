// XXX Automatically generated. DO NOT EDIT! XXX //

public:
AddressList();
AddressList(const AddressList &);
AddressList(const QCString &);
AddressList & operator = (const AddressList &);
AddressList & operator = (const QCString &);
friend QDataStream & operator >> (QDataStream & s, AddressList &);
friend QDataStream & operator << (QDataStream & s, AddressList &);
bool operator == (AddressList &);
bool operator != (AddressList & x) { return !(*this == x); }
bool operator == (const QCString & s) { AddressList a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~AddressList();
virtual bool isNull() { parse(); return strRep_.isEmpty(); }
virtual bool operator ! () { return isNull(); }
virtual void createDefault();

virtual const char * className() const { return "AddressList"; }

protected:
virtual void _parse();
virtual void _assemble();

// End of automatically generated code           //

// XXX Automatically generated. DO NOT EDIT! XXX //

public:
Mailbox();
Mailbox(const Mailbox &);
Mailbox(const QCString &);
Mailbox & operator = (const Mailbox &);
Mailbox & operator = (const QCString &);
friend QDataStream & operator >> (QDataStream & s, Mailbox &);
friend QDataStream & operator << (QDataStream & s, Mailbox &);
bool operator == (Mailbox &);
bool operator != (Mailbox & x) { return !(*this == x); }
bool operator == (const QCString & s) { Mailbox a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~Mailbox();
virtual bool isNull() { parse(); return strRep_.isEmpty(); }
virtual bool operator ! () { return isNull(); }
virtual void createDefault();

virtual const char * className() const { return "Mailbox"; }

protected:
virtual void _parse();
virtual void _assemble();

// End of automatically generated code           //

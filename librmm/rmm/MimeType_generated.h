// XXX Automatically generated. DO NOT EDIT! XXX //

public:
MimeType();
MimeType(const MimeType &);
MimeType(const QCString &);
MimeType & operator = (const MimeType &);
MimeType & operator = (const QCString &);
friend QDataStream & operator >> (QDataStream & s, MimeType &);
friend QDataStream & operator << (QDataStream & s, MimeType &);
bool operator == (MimeType &);
bool operator != (MimeType & x) { return !(*this == x); }
bool operator == (const QCString & s) { MimeType a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~MimeType();
virtual bool isNull() { parse(); return strRep_.isEmpty(); }
virtual bool operator ! () { return isNull(); }
virtual void createDefault();

virtual const char * className() const { return "MimeType"; }

protected:
virtual void _parse();
virtual void _assemble();

// End of automatically generated code           //

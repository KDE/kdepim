// XXX Automatically generated. DO NOT EDIT! XXX //

public:
ContentType();
ContentType(const ContentType &);
ContentType(const QCString &);
ContentType & operator = (const ContentType &);
ContentType & operator = (const QCString &);
friend QDataStream & operator >> (QDataStream & s, ContentType &);
friend QDataStream & operator << (QDataStream & s, ContentType &);
bool operator == (ContentType &);
bool operator != (ContentType & x) { return !(*this == x); }
bool operator == (const QCString & s) { ContentType a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~ContentType();
virtual bool isNull() { parse(); return strRep_.isEmpty(); }
virtual bool operator ! () { return isNull(); }
virtual void createDefault();

virtual const char * className() const { return "ContentType"; }

protected:
virtual void _parse();
virtual void _assemble();

// End of automatically generated code           //

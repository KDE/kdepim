// XXX Automatically generated. DO NOT EDIT! XXX //

public:
ContentDisposition();
ContentDisposition(const ContentDisposition &);
ContentDisposition(const QCString &);
ContentDisposition & operator = (const ContentDisposition &);
ContentDisposition & operator = (const QCString &);
friend QDataStream & operator >> (QDataStream & s, ContentDisposition &);
friend QDataStream & operator << (QDataStream & s, ContentDisposition &);
bool operator == (ContentDisposition &);
bool operator != (ContentDisposition & x) { return !(*this == x); }
bool operator == (const QCString & s) { ContentDisposition a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~ContentDisposition();
virtual bool isNull() { parse(); return strRep_.isEmpty(); }
virtual bool operator ! () { return isNull(); }
virtual void createDefault();

virtual const char * className() const { return "ContentDisposition"; }

protected:
virtual void _parse();
virtual void _assemble();

// End of automatically generated code           //

// XXX Automatically generated. DO NOT EDIT! XXX //

public:
Text();
Text(const Text &);
Text(const QCString &);
Text & operator = (const Text &);
Text & operator = (const QCString &);
friend QDataStream & operator >> (QDataStream & s, Text &);
friend QDataStream & operator << (QDataStream & s, Text &);
bool operator == (Text &);
bool operator != (Text & x) { return !(*this == x); }
bool operator == (const QCString & s) { Text a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~Text();
virtual bool isNull() { parse(); return strRep_.isEmpty(); }
virtual bool operator ! () { return isNull(); }
virtual void createDefault();

virtual const char * className() const { return "Text"; }

protected:
virtual void _parse();
virtual void _assemble();

// End of automatically generated code           //

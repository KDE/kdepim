// XXX Automatically generated. DO NOT EDIT! XXX //

public:
BodyPart();
BodyPart(const BodyPart &);
BodyPart(const QCString &);
BodyPart & operator = (const BodyPart &);
BodyPart & operator = (const QCString &);
friend QDataStream & operator >> (QDataStream & s, BodyPart &);
friend QDataStream & operator << (QDataStream & s, BodyPart &);
bool operator == (BodyPart &);
bool operator != (BodyPart & x) { return !(*this == x); }
bool operator == (const QCString & s) { BodyPart a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~BodyPart();
virtual bool isNull() { parse(); return strRep_.isEmpty(); }
virtual bool operator ! () { return isNull(); }
virtual void createDefault();

virtual const char * className() const { return "BodyPart"; }

protected:
virtual void _parse();
virtual void _assemble();

// End of automatically generated code           //

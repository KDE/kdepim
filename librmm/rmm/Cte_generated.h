// XXX Automatically generated. DO NOT EDIT! XXX //

public:
Cte();
Cte(const Cte &);
Cte(const QCString &);
Cte & operator = (const Cte &);
Cte & operator = (const QCString &);
friend QDataStream & operator >> (QDataStream & s, Cte &);
friend QDataStream & operator << (QDataStream & s, Cte &);
bool operator == (Cte &);
bool operator != (Cte & x) { return !(*this == x); }
bool operator == (const QCString & s) { Cte a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~Cte();
virtual bool isNull() { parse(); return strRep_.isEmpty(); }
virtual bool operator ! () { return isNull(); }
virtual void createDefault();

virtual const char * className() const { return "Cte"; }

protected:
virtual void _parse();
virtual void _assemble();

// End of automatically generated code           //

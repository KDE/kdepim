// XXX Automatically generated. DO NOT EDIT! XXX //

RHeaderBody();
RHeaderBody(const RHeaderBody &);
RHeaderBody(const QCString &);
virtual RHeaderBody & operator = (const RHeaderBody &);
virtual RHeaderBody & operator = (const QCString &);
virtual bool operator == (RHeaderBody &);
virtual bool operator != (RHeaderBody & x) { return !(*this == x); }
virtual bool operator == (const QCString & s) { RHeaderBody a(s); return (*this == a); } 
virtual bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RHeaderBody();
virtual void _parse();
virtual void _assemble();
virtual void parse() 			{ if (!parsed_) _parse(); parsed_ = true; assembled_ = false; }

virtual void assemble() 			{ parse() ; if (!assembled_) _assemble(); assembled_ = true;}

virtual void createDefault();

virtual const char * className() const { return "RHeaderBody"; }

// End of automatically generated code           //

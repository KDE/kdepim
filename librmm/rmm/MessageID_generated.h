// XXX Automatically generated. DO NOT EDIT! XXX //

public:
MessageID();
MessageID(const MessageID &);
MessageID(const QCString &);
MessageID & operator = (const MessageID &);
MessageID & operator = (const QCString &);
friend QDataStream & operator >> (QDataStream & s, MessageID &);
friend QDataStream & operator << (QDataStream & s, MessageID &);
bool operator == (MessageID &);
bool operator != (MessageID & x) { return !(*this == x); }
bool operator == (const QCString & s) { MessageID a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~MessageID();
virtual bool isNull() { parse(); return strRep_.isEmpty(); }
virtual bool operator ! () { return isNull(); }
virtual void createDefault();

virtual const char * className() const { return "MessageID"; }

protected:
virtual void _parse();
virtual void _assemble();

// End of automatically generated code           //

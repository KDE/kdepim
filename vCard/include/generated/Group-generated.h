// XXX Automatically generated. DO NOT EDIT! XXX //

public:
V_Group();
V_Group(const V_Group&);
V_Group(const QCString&);
V_Group & operator = (V_Group&);
V_Group & operator = (const QCString&);
bool operator ==(V_Group&);
bool operator !=(V_Group& x) {return !(*this==x);}
bool operator ==(const QCString& s) {V_Group a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~V_Group();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();

// End of automatically generated code           //

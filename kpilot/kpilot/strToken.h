#ifndef __STRING_TOKENIZER_H
#define __STRING_TOKENIZER_H

class StrTokenizer
    {
    public:
    StrTokenizer(const char* string, const char* delims);
    ~StrTokenizer() { delete [] fOrigString; delete [] fDelims; }
    
    const char* getNextField();
    
    protected:
    char* fOrigString;
    char* fString;
    char* fDelims;
    };

#endif

#include "strToken.h"
#include <string.h>

StrTokenizer::StrTokenizer(const char* string, const char* delims)
    {
    fOrigString = fString = new char[strlen(string) + 1];
    strcpy(fString, string);
    fDelims = new char[strlen(delims) + 1];
    strcpy(fDelims, delims);
    }

const char*
StrTokenizer::getNextField()
    {
    char* strStart = fString;

    if(*fString == 0L)
	return 0L;
    while(*fString && !(strchr(fDelims, *fString)))
	fString++;
    if(*fString)
	{
	*fString = 0L;
	fString++;
	}
    return strStart;
    }

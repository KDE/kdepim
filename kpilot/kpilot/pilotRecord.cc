#include <string.h>
#include "pilotRecord.h"

PilotRecord::PilotRecord(void* data, int len, int attrib, int cat, pi_uid_t uid)
  : fData(0L), fLen(len), fAttrib(attrib), fCat(cat), fID(uid)
    {
    fData = new char[len];
    memcpy(fData, data, len);
    }

PilotRecord::PilotRecord(PilotRecord* orig)
    {
    fData = new char[orig->getLen()];
    memcpy(fData, orig->getData(), orig->getLen());
    fLen = orig->getLen();
    fAttrib = orig->getAttrib();
    fCat = orig->getCat();
    fID = orig->getID();
    }

PilotRecord& PilotRecord::operator=(PilotRecord& orig)
    {
    if(fData)
	delete [] fData;
    fData = new char[orig.getLen()];
    memcpy(fData, orig.getData(), orig.getLen());
    fLen = orig.getLen();
    fAttrib = orig.getAttrib();
    fCat = orig.getCat();
    fID = orig.getID();
    return *this;
    }

void PilotRecord::setData(const char* data, int len)
    {
    if(fData)
	delete [] fData;
    fData = new char[len];
    memcpy(fData, data, len);
    fLen = len;
    }

bool PilotRecord::isDeleted() const
{
	return getAttrib() & dlpRecAttrDeleted ;
}

bool PilotRecord::isSecret() const
{
	return getAttrib() & dlpRecAttrSecret ;
}


void PilotRecord::makeDeleted()
{
	fAttrib |= dlpRecAttrDeleted ;
}



void PilotRecord::makeSecret()
{
	fAttrib |= dlpRecAttrSecret ;
}


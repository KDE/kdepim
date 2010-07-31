#include "intid.h"

#include<kdebug.h>

KornIntId::KornIntId(int id) : _id(id)
{
}

KornIntId::KornIntId(const KornIntId& src) : KornMailId(), _id(src._id)
{
}

KornIntId::~KornIntId()
{
}

TQString KornIntId::toString() const
{
	return TQString("KornIntId, Id: ") + TQString::number(_id);
}

KornMailId * KornIntId::clone() const
{
	return new KornIntId(*this);
}

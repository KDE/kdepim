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

QString KornIntId::toString() const
{
	return QString("KornIntId, Id: ") + QString::number(_id);
}

KornMailId * KornIntId::clone() const
{
	return new KornIntId(*this);
}

#ifndef STRINGID_H
#define STRINGID_H

/*
 * This class provides a identification with string,
 * because in KIO, I don't know if emails return in the same order.
 * Author Mart Kelder
 */

#include<qstring.h>
#include"mailid.h"

class KornStringId : public KornMailId
{
private:
	QString _id;
public:
	KornStringId( const QString & id );
	KornStringId( const KornStringId & src );
	~KornStringId() {}

	QString getId() const { return _id; }
	virtual QString toString() const { return _id; }

	virtual KornMailId *clone() const;
};

#endif //STRINGID_H

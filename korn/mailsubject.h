#ifndef MailSubject_h
#define MailSubject_h

class KMailDrop;

#include "mailid.h"



/**
 * A KornMailSubject instance represents a single mail. It stores 
 * its id, sender, subject, header, size and date and possibly its
 * mail body
 */
class KornMailSubject
{
	KornMailId * 	_id;
	KMailDrop *     _drop;
	QString		_subject;
	QString		_sender;
	QString		_header;
	int 		_size;
	int		_date;
	bool 		_fullMessage;
public:
	/**
	 * KornMailSubject default constructor
	 */
	KornMailSubject();

	/**
	 * KornMailSubject constructor
	 * @param id id of the mail. The KornMailId instance should
	 * not be touched or deleted afterwards. It is destroyed by
	 * KornMailSubject's destructor.
	 * @param drop The KMailDrop.
	 */
	KornMailSubject(KornMailId * id, KMailDrop * drop);

	/**
	 * KornMailSubject copy constructor. All data of the source
	 * KornMailSubject instance are cloned.
	 * @param src KornMailSubject to copy from
	 */
	KornMailSubject(const KornMailSubject & src);

	/**
	 * replaces the contents of this by the contents of another
	 * KornMailSubject instance.  All data of the source
	 * KornMailSubject instance are cloned.
	 * @param src KornMailSubject to copy from
	 */
	KornMailSubject & operator= (const KornMailSubject & src);
	
	/**
	 * KornMailSubject destructor
	 */
	virtual ~KornMailSubject();

	/**
	 * Return the mail id.
	 * @return the mail id.
	 */
	const KornMailId * getId() const {return _id;}

	/**
	 * Set the mails subject.
	 * @param subject the mails subject.
	 */
	void setSubject(const QString & subject) {_subject = subject;}

	/**
	 * Return the subject.
	 * @return the subject.
	 */
	const QString & getSubject() const {return _subject;}

	/**
	 * Set the mails sender.
	 * @param sender the mails sender.
	 */
	void setSender(const QString & sender) {_sender = sender;}

	/**
	 * Return the sender.
	 * @return the sender.
	 */
	const QString & getSender() const {return _sender;}

	/**
	 * Set the mails header and (if possible) body.
	 * @param header the mails header with or without body (see fullMessage parameter).
	 * @param fullMessage true, if header contains the message body as well, false otherwise.
	 */
	void setHeader(const QString & header, bool fullMessage) {_header = header; _fullMessage = fullMessage;}

	/**
	 * Return the header or the full message (if isHeaderFullMessage() is true).
	 * @return the header or the full message.
	 */
	const QString & getHeader() const {return _header;}

	/**
	 * Return true, if the header contains the header and the full message.
	 * Return false if the header just contains the header.
	 * @return see above.
	 */
	bool isHeaderFullMessage() const {return _fullMessage;}

	/**
	 * Set the size of the full mail.
	 * @param the mails full size.
	 */
	void setSize(int size) {_size = size;}

	/**
	 * Return the size of the full mail.
	 * @return the size of the full mail.
	 */
	const int getSize() const {return _size;}

	/**
	 * Set the mails date in seconds since 1970-01-01 00:00:00.
	 * @param date the mails date.
	 */
	void setDate(int date) {_date = date;}

	/**
	 * Return the mails date in seconds since 1970-01-01 00:00:00.
	 * @return the mails date.
	 */
	const int getDate() const {return _date;}

	/**
	 * Return a string representation of this (for debugging purposes only)
	 * @return a string representation
	 */
	QString toString() const;
	
	/**
	 * Sets the KMailDrop field.
	 * @param drop The KMailDrop-object
	 */
	void setMailDrop( KMailDrop* drop ) { _drop = drop; }
	
	/**
	 * Returns the KMailDrop instance of the Maildrop which owns the subject
	 */
	KMailDrop* getMailDrop() const { return _drop; }
};

#endif

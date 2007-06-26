/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/
#include "sms.h"
#include <qstringlist.h>
#include <qdatetime.h>
#include <kdebug.h>
#include <qdir.h>
#include <qfile.h>
#include <klocale.h>
#include <qregexp.h>
#include <kcodecs.h>
#include <QTextStream>
#define SMS_MIMETYPE "application/x-kmobiletools-sms"

#include "kmobiletoolshelper.h"

using namespace KMobileTools;

class SMSPrivate {
    public:
        SMSPrivate(SMS* p_parent) :
        i_folder(0), i_slot(0)
        { parent=p_parent; }
    QStringList sl_numbers;
//     QString s_text;
    int i_folder;
    int i_slot;
    QList<int> v_id;
    QString s_rawSlot;
    SMS *parent;
    QString s_uid;
    void refreshUid()
    {
        KMD5 context;
        QByteArray ba;
        if ( sl_numbers.isEmpty()) ba = parent->body();
        else ba = ( parent->getText() + sl_numbers.join(",")).toUtf8();
        context.update(ba);
//         s_uid=QString(context.hexDigest() );
    }
};
class SenderPrivate {
public:
    SenderPrivate() {}
    PhoneNumbers numbers;
    QByteArray numberAs7BitString( QHash<QString,QString>::ConstIterator n ) const
    {
        if( n.value().isEmpty() ) return n.key().toUtf8();
        // adapted from kmime_header_parsing.cpp
        // Copyright (c) 2001-2002 Marc Mutz <mutz@kde.org>
        QByteArray rv;
        if ( KMime::isUsAscii( n.value() ) ) {
            QByteArray tmp = n.value().toLatin1();
            KMime::addQuotes( tmp, false );
            rv += tmp;
        } else {
            rv += KMime::encodeRFC2047String( n.value(), QByteArray() /** @TODO verify this */, true );
        }
        rv += " <" + n.key().toUtf8() + '>';
        return rv;
    }
    bool parsePhoneNumberString(const QString &s)
    {
        /// @TODO handle more charset
        kDebug() << k_funcinfo << "() " << s << endl;
        QString number, name;
        QRegExp src;
        src.setMinimal(true);
        if(! s.contains('\"') )
            src.setPattern("(.*)<(.*)>");
        else src.setPattern("^\"(.*)\"[\\s]*<(.*)>");
        kDebug() << "Regexp Pattern: " << src.pattern() << endl;
        if( src.indexIn( s.trimmed() )==-1 ) { kDebug() << " search failed\n"; return false;}
        numbers[src.cap(2)]=src.cap(1);
        kDebug() << "adding contact: <<" << src.cap(1).trimmed() << ">> <<" << src.cap(2).trimmed() << ">>\n";
        return true;
    }
};
class DestinationPrivate {
public:
    DestinationPrivate() {}
};

Sender::Sender() : KMime::Headers::Generics::Structured(), d(new SenderPrivate) {
}

Sender::~Sender() {}

const char *Sender::type() const { return "Sender"; }

void Sender::addNumber(const QString &number, const QString &displayname )
{
    d->numbers[number]=displayname;
}

void Sender::clear() { d->numbers.clear(); }
bool Sender::isEmpty() const { return d->numbers.isEmpty(); }

bool Sender::parse(const char *&scursor, const char *const send, bool isCRLF)
{
/// @TODO handle more charset
    kDebug() << k_funcinfo << "() " << scursor << ", " << int(send-scursor) << ", " << isCRLF << endl;
//     return false;
    QStringList singlenums;
    // strip out type()
    QString stype=QString("%1: ").arg(type() );
    if (stype==QString(scursor).left(stype.size() ) )
        scursor=&scursor[stype.size() ];
    else return false;
    bool hadQuotes=false;
    char *start=(char*) scursor;
    for( int i=0; i<(send-scursor); i++){
//         kDebug() << "checking char '" << scursor[i] << "' (" << int(scursor[i]) << ")\n";
        if(scursor[i]=='"' && scursor[i-1]!='\\')
            hadQuotes=!hadQuotes;
        if( !hadQuotes && scursor[i]==',' )
        {
            singlenums+=QString(start).left(i-(start-scursor));
            start=(char*) &scursor[i+1];
        }
    }
    singlenums+=QString(const_cast<char*>(start) );
//     kDebug() << "Single nums found: >>>\n" << singlenums.join("|||\n|||") << "|||;" << endl;
    bool ret=true;
    for( int i=0; i<singlenums.size(); i++ )
        ret&=d->parsePhoneNumberString( singlenums.at(i) );
    return ret;
}

QByteArray Sender::as7BitString(bool withHeaderType) const
{
    kDebug() << k_funcinfo << "() " << withHeaderType << endl;
    QByteArray ret;
    if( isEmpty() ) return ret;
    if(withHeaderType) {
        ret+=type();
        ret+=": ";
    }
    for( QHash<QString,QString>::ConstIterator it=phoneNumbers().constBegin(); it!=phoneNumbers().constEnd(); ++it){
        ret+=d->numberAs7BitString(it);
        ret+=", ";
    }
    ret.resize(ret.length() -2);
    return ret;
}

PhoneNumbers Sender::phoneNumbers() const
{
    return d->numbers;
}

Destination::Destination() : Sender(), d(new DestinationPrivate) {
}
const char *Destination::type() const { return "Destination"; }


SMS::SMS() : KMime::Content(),
    d(new SMSPrivate(this) )
{
}

SMS::SMS(const QStringList & numbers, const QString & text, const KDateTime & datetime)
 : KMime::Content(), d(new SMSPrivate(this) )
{
    setNumbers(numbers);
    setText(text);
    setDateTime(datetime);
    setFolder(d->i_folder);
}

QString SMS::uid() const {
    return d->s_uid;
}

SMS::SMS(const QStringList & numbers, const QString & text)
 : KMime::Content(), d(new SMSPrivate(this) )
{
    setNumbers(numbers);
    setText(text);
}

SMS::~SMS()
{
}

QStringList SMS::getTo() const
{
    if (type()==Unsent || type()==Sent)
    {
        return d->sl_numbers;
    } else {
        return QStringList();
    }
}


bool SMS::operator ==( SMS* compSMS)
{
    return (compSMS->uid() == this->uid());
}

int SMS::getMultiTextCount(const QString &text)
{
    return getMultiTextCount(text.length());
}

int SMS::getMultiTextCount(int length)
{
    if(length <= 160) return 1;
    int textLength=0;
    if(length <= 1404 ) textLength=156; else textLength=154;
    if(length % textLength) return (length / textLength)+1;
    else return (length / textLength);
}

bool SMS::isIncoming() const { return ((type() & Unread) || (type() & Read)); }

QStringList SMS::getMultiText(const QString &text)
{
    if(text.length()<=160 ) return QStringList(text);
    QStringList out;
    /* Now some notes about total text calculation
    We're adding a separator at the beginning of each message: "n/m:" (without quotes, but WITH the semicolon).
    Now.. if m<10, the separator steal us 4 characters for each sms, so we must split the message in strings of 156 characters.
    So we should check if message length is <=1404 characters.
    If message length > 1404 characters, we should use a "nn/mm:" notation, and now the separarator steal us 6 characters.
    Each message part now should be 154 characters, and so our maximal length now is 15246 characters.
    I just hope that in this world nobody is so mad to send a multi-message made with more than 99 SMS ;-)
    */
    int sepLength, textLength,minLength;
    const QString sep="%1/%2:";
    int totalMessages;
    if(text.length() <= 1404 )
    {
        sepLength=4;
        textLength=156;
        minLength=1;
    } else
    {
        sepLength=6;
        textLength=154;
        minLength=2;
    }
    if(text.length() % textLength) totalMessages=(text.length()/ textLength) +1;
    else totalMessages=text.length() / textLength;
    int j=0;
    for(int i=0; i<text.length(); i+=textLength)
    {
        j++;
        out+=text.mid( i, textLength )
                .prepend( sep
                        .arg( j, minLength )
                        .arg( totalMessages, minLength )
                        .replace( ' ','0') );
    }
    return out;
}

// Convenience non-static method for the above one
QStringList SMS::getMultiText() const
{
    d->refreshUid(); /// @TODO move this to the single setters?
    return getMultiText(getText() );
}


/*!
    \fn SMS::export(const QString &dir)
 */
bool SMS::exportMD(const QString &dir)
{
    bool retval=false;
    if (d->i_slot & SIM )
        retval = retval | writeToSlot( dir + QDir::separator() + '.' + i18nc("SIM MailDir", "SIM") + ".directory");
    if (d->i_slot & Phone )
        retval = retval | writeToSlot( dir + QDir::separator() + '.' + i18nc("Phone MailDir", "Phone") + ".directory");
    return retval;
}

bool SMS::writeToSlot(const QString &dir)
{
    QString filename=dir+QDir::separator();
    QString text;
    if((type() & Unsent) || (type() & Sent) )
    {
        filename+=i18nc("Outgoing MailDir", "Outgoing");
        text="To: \"";
        for(QStringList::Iterator it=d->sl_numbers.begin(); it!=d->sl_numbers.end(); ++it)
            text+=KMobileTools::KMobiletoolsHelper::translateNumber(*it) + "\" <" + *it + ">\n";
        text+="X-Status: RS\n";
    }
    else
    {
        filename+=i18nc("Incoming MailDir", "Incoming");
        text="From: \"" + KMobileTools::KMobiletoolsHelper::translateNumber( getFrom() ) + "\" <" + getFrom() + ">\n";
        text+="X-Status: RC\n";
    }
    QString subject=i18nc("SMS/Mail Subject", "[KMobileTools Imported Message]") + ' ';
    subject+=KCodecs::quotedPrintableEncode(
                    getText().left( 20+ getText().indexOf( ( QRegExp("[\\s]"), 20 )) ).toUtf8() );
    subject+="...";
    subject=subject.replace( QRegExp("([^\\s]*=[\\dA-F]{2,2}[^\\s]*)"), "=?utf-8?q?\\1?=");
    text+=QString("Subject: %1\n").arg(subject);
    text+="Date: " + getDateTime().toString( "%1, d %2 yyyy hh:mm:ss" )
            .arg( KMobileTools::KMobiletoolsHelper::shortWeekDayNameEng( getDateTime().date().dayOfWeek() ) )
            .arg( KMobileTools::KMobiletoolsHelper::shortMonthNameEng( getDateTime().date().month() ) )
            + '\n';
    text+="X-KMobileTools-IntType: " + QString::number(type() ) + '\n';
    text+="X-KMobileTools-TextType: " + SMSTypeString(type() ) + '\n';
    text+="X-KMobileTools-PhoneNumbersTo: " + d->sl_numbers.join(",") + '\n';
    text+="X-KMobileTools-PhoneNumbersFrom: " + getFrom() + '\n';
    text+="X-KMobileTools-RawSlot: " + rawSlot() + '\n';
    text+="Content-Type: text/plain;\n \tcharset=\"utf-8\"\nContent-Transfer-Encoding: quoted-printable\n";

//     text+="Content-Type: text/plain; charset=utf-8\n";
    text+="\n\n" + KCodecs::quotedPrintableEncode( getText().toUtf8() )+ '\n';
    filename=filename + QDir::separator() + "cur" + QDir::separator() +
            QString::number(getDateTime().toTime_t()) + '.' + QString(uid()) + '.' + "kmobiletools";
    kDebug() << "Writing sms to " << filename << endl;
    QFile file(filename);
    if(! file.open( QIODevice::WriteOnly | QIODevice::Truncate ) ) return false;
    QTextStream stream( &file );
    stream << text;
    file.close();
    return true;
}

/*!
    \fn SMS::exportCSV(const QString &dir)
 */
bool SMS::exportCSV(const QString &filename)
{
    kDebug() << k_funcinfo << endl;
    bool retval=false;
    if (d->i_slot & Phone )
        retval = retval | writeToSlotCSV( filename );
    return retval;
}

bool SMS::writeToSlotCSV(const QString &filename)
{
    kDebug() << k_funcinfo << endl;
    QString text;

    if((type() & Unsent) || (type() & Sent) )
    {
        text="\"OUTGOING\",";
        for(QStringList::Iterator it=d->sl_numbers.begin(); it!=d->sl_numbers.end(); ++it)
            text+="\"" + KMobileTools::KMobiletoolsHelper::translateNumber(*it) + "\",\"" + *it + "\",";
    }
    else
    {
        QString transNumber;
        //transNumber = KMobileTools::KMobiletoolsHelper::translateNumber( getFrom() ).utf8();
        transNumber = KMobileTools::KMobiletoolsHelper::translateNumber( getFrom() );

        //text="\"INCOMING\",\"" + KMobileTools::KMobiletoolsHelper::translateNumber( getFrom() ) + "\",\"" + getFrom() + "\",";
        text="\"INCOMING\",\"" + transNumber  + "\",\"" + getFrom() + "\",";
    }

    text+="\"" + getDateTime().toString( "%1, d %2 yyyy hh:mm:ss" )
            .arg( KMobileTools::KMobiletoolsHelper::shortWeekDayNameEng( getDateTime().date().dayOfWeek() ) )
            .arg( KMobileTools::KMobiletoolsHelper::shortMonthNameEng( getDateTime().date().month() ) )
            + "\",";
    //text+="\"" + KCodecs::quotedPrintableEncode( getText().utf8() ) + "\"";
    //text+="\"" + getText().utf8() + "\"";
    text+="\"" + getText() + "\"";
    kDebug() << "Writing sms to " << filename << endl;
    QFile file(filename);
    QString lastFile = file.readAll();
    if(! file.open( QIODevice::WriteOnly | QIODevice::Append ) ) return false;
    QTextStream stream( &file );
    //stream << lastFile << text.utf8() << endl;
    stream << lastFile << text << endl;
    file.close();
    return true;
}

void SMS::setText(const QString & text) { setBody(text.toUtf8()); }
QString SMS::getText() const { return QString(body() ); }
QString SMS::getDate() const { return getDateTime().toString(); }

KDateTime SMS::getDateTime() const {
    if(! const_cast<SMS*>(this)->hasHeader("Date") ) return KDateTime();
    return (const_cast<SMS*>(this)->date() )->dateTime();
}
void SMS::setDateTime(const KDateTime & datetime) {
    KMime::Headers::Date *h=new KMime::Headers::Date();
    h->setDateTime(datetime);
    setHeader(h);
}

bool SMS::unread() const {
    return const_cast<SMS*>(this)->hasHeader("X-KMobileTools-Read");
}

void SMS::setUnread(bool unread) {
    if(unread && ! this->unread() ) {
        removeHeader("X-KMobileTools-Read");
        return;
    }
    setHeader( new KMime::Headers::Generic("X-KMobileTools-Read", 0, QString("read").toUtf8() ) );
}

QString SMS::SMSTypeString(SMSType smstype) {
    switch (smstype) {
        case SMS::Unread:
        return "REC UNREAD";
        case SMS::Read:
        return "REC READ";
        case SMS::Unsent:
        return "STO UNSENT";
        case SMS::Sent:
        return  "STO SENT";
        case SMS::All:
        return "ALL";
    }
    return QString();
}

SMS::SMSType SMS::SMSIntType(const QString& type) {
    if (type==QLatin1String("REC UNREAD")) return SMS::Unread;
    if (type==QLatin1String("REC READ")) return SMS::Read;
    if (type==QLatin1String("STO UNSENT")) return SMS::Unsent;
    if (type==QLatin1String("STO SENT")) return SMS::Sent;
    if (type==QLatin1String("ALL")) return SMS::Sent;
    return SMS::All; // Good enough or switch back to -1?
}

QByteArray SMS::assembleHeaders()
{
    KMime::Headers::Base *h;
    QByteArray ret;
    h=date();
    if(h) ret+= h->as7BitString()+'\n';
    h=sender();
    if(h) ret+= h->as7BitString()+'\n';
    h=destination();
    if(h) ret+= h->as7BitString()+'\n';
    h=getHeaderByType("X-KMobileTools-Type");
    if(h) ret+= h->as7BitString()+'\n';
    h=getHeaderByType("X-KMobileTools-Read");
    if(h) ret+= h->as7BitString()+'\n';
    return ret + Content::assembleHeaders();
}

// Headers implementation
KMime::Headers::Date *SMS::date() const{
    return dynamic_cast<KMime::Headers::Date*>(const_cast<SMS*>(this)->getHeaderByType("Date") );
}

void SMS::setSender(const QString& number, const QString &displayname) {
    Sender *fromh=sender();
    if(!fromh) fromh=new Sender();
    fromh->clear();
    fromh->addNumber(number, displayname);
    setHeader(fromh);
}

void SMS::addDestination(const QString& number, const QString &displayname) {
    Destination * h=destination();
    if(!h) {
        h=new Destination;
        setHeader(h);
    }
    h->addNumber(number.toUtf8(), displayname);
}

Sender *SMS::sender() const
{
    return dynamic_cast<Sender*>(const_cast<SMS*>(this)->getHeaderByType("Sender") );
}

Destination *SMS::destination() const
{
    return dynamic_cast<Destination*>(const_cast<SMS*>(this)->getHeaderByType("Destination") );
}

QString SMS::getFrom() const
{
    if (type()==Unsent || type()==Sent)
    {
        return QString();
    } else {
        return QString( d->sl_numbers.first() );
    }
}

SMS::SMSType SMS::type() const {
    KMime::Headers::Generic* htype=dynamic_cast<KMime::Headers::Generic*>(const_cast<SMS*>(this)->getHeaderByType("X-KMobileTools-Type") );
    if(!htype) return All; /// @TODO error handling here
    kDebug() << htype->as7BitString (false);
    return SMSIntType( QString ( htype->as7BitString() ) );
}
void SMS::setType( SMSType newType ) {
    KMime::Headers::Generic *htype=dynamic_cast<KMime::Headers::Generic*>(getHeaderByType("X-KMobileTools-Type") );
    if(!htype) htype=new KMime::Headers::Generic("X-KMobileTools-Type");
    htype->from7BitString( SMS::SMSTypeString(newType).toUtf8() );
    setHeader( htype );
}


/// @TODO port-or-delete the following ones
void SMS::setRawSlot(const QString &rawSlot){ d->s_rawSlot=rawSlot;}
QString SMS::rawSlot() const { return d->s_rawSlot;}
void SMS::setNumbers(const QStringList & numbers) { d->sl_numbers=numbers; }
void SMS::setFolder( int newFolder ) { d->i_folder = newFolder; }
int SMS::folder() const { return d->i_folder; }
QList<int> *SMS::idList() { return &(d->v_id); }
void SMS::setSlot( int newSlot ) { d->i_slot=newSlot; /*emit updated();*/ }

int SMS::slot() const { return d->i_slot; }



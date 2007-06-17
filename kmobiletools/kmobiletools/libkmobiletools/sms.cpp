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
#include "kmobiletoolshelper.h"

class SMSPrivate {
    public:
        SMSPrivate(SMS* p_parent)
    : i_folder(0), i_slot(0), i_type(SMS::All), b_unread(false)
        { parent=p_parent; }
    QStringList sl_numbers;
    QString s_text;
    QDateTime dt_datetime;
    int i_folder;
    int i_slot;
    SMS::SMSType i_type;
    QList<int> v_id;
    QString s_rawSlot;
    bool b_unread;
    SMS *parent;
    void refreshUid() const
    {
        KMD5 context;
        QByteArray ba;
        if ( sl_numbers.isEmpty()) ba = s_text.toUtf8();
        else ba = ( s_text + sl_numbers.join(",")).toUtf8();
        context.update(ba);
        parent->reference().setRemoteId( context.hexDigest() );
    }
};

SMS::SMS(QObject *parent)
 : QObject(parent), 
  Akonadi::Item("text/sms"), /// @TODO could this a proper mimetype? eventually add it in kde global mimetypes.
  d(new SMSPrivate(this) )
{
}

SMS::SMS(const QStringList & numbers, const QString & text, const QDateTime & datetime, QObject *parent)
 : QObject(parent), d(new SMSPrivate(this) )
{
    setNumbers(numbers);
    setText(text);
    setDateTime(datetime);
    setFolder(d->i_folder);
}

QString SMS::uid() const {
    return reference().remoteId();
}

SMS::SMS(const QStringList & numbers, const QString & text, QObject *parent)
 : QObject(parent), d(new SMSPrivate(this) )
{
    setNumbers(numbers);
    setText(text);
}

void SMS::setDateTime(const QDateTime & datetime)
{
    d->dt_datetime=datetime;
}

SMS::~SMS()
{
}

QString SMS::getFrom() const
{
    if (d->i_type==Unsent || d->i_type==Sent)
    {
        return QString();
    } else {
        return QString( d->sl_numbers.first() );
    }
}

QStringList SMS::getTo() const
{
    if (d->i_type==Unsent || d->i_type==Sent)
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
    return getMultiText(d->s_text);
}

#include "sms.moc"


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
    if((d->i_type & Unsent) || (d->i_type & Sent) )
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
    text+="Date: " + d->dt_datetime.toString( "%1, d %2 yyyy hh:mm:ss" )
            .arg( KMobileTools::KMobiletoolsHelper::shortWeekDayNameEng( d->dt_datetime.date().dayOfWeek() ) )
            .arg( KMobileTools::KMobiletoolsHelper::shortMonthNameEng( d->dt_datetime.date().month() ) )
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
            QString::number(d->dt_datetime.toTime_t()) + '.' + QString(uid()) + '.' + "kmobiletools";
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

    if((d->i_type & Unsent) || (d->i_type & Sent) )
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

    text+="\"" + d->dt_datetime.toString( "%1, d %2 yyyy hh:mm:ss" )
            .arg( KMobileTools::KMobiletoolsHelper::shortWeekDayNameEng( d->dt_datetime.date().dayOfWeek() ) )
            .arg( KMobileTools::KMobiletoolsHelper::shortMonthNameEng( d->dt_datetime.date().month() ) )
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

void SMS::setText(const QString & text) { d->s_text=text; }
QString SMS::getText() const { return d->s_text; }
QString SMS::getDate() const { return d->dt_datetime.toString(); }
QDateTime SMS::getDateTime() const { return d->dt_datetime; }
void SMS::setRawSlot(const QString &rawSlot){ d->s_rawSlot=rawSlot;}
QString SMS::rawSlot() const { return d->s_rawSlot;}
void SMS::setNumbers(const QStringList & numbers) { d->sl_numbers=numbers; }
void SMS::setFolder( int newFolder ) { d->i_folder = newFolder; }
int SMS::folder() const { return d->i_folder; }
QList<int> *SMS::idList() { return &(d->v_id); }
void SMS::setSlot( int newSlot ) { d->i_slot=newSlot; emit updated(); }
SMS::SMSType SMS::type() const { return d->i_type; }
void SMS::setType( SMSType newType ) { d->i_type = newType; emit updated(); }
int SMS::slot() const { return d->i_slot; }
bool SMS::unread() const { return d->b_unread; }
void SMS::setUnread(bool unread) { d->b_unread=unread;}

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

int SMS::SMSIntType(const QString& type) {
    if (type==QLatin1String("REC UNREAD")) return SMS::Unread;
    if (type==QLatin1String("REC READ")) return SMS::Read;
    if (type==QLatin1String("STO UNSENT")) return SMS::Unsent;
    if (type==QLatin1String("STO SENT")) return SMS::Sent;
    if (type==QLatin1String("ALL")) return SMS::Sent;
    return -1;
}

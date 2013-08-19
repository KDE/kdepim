//
//  Copyright (C) 2005 - 2011 Kevin Krammer <kevin.krammer@gmx.at>
//  Copyright (C) 2011 Fernando Schapachnik <fernando@schapachnik.com.ar>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

// local includes
#include "outputformatimpls.h"
#include "csvtemplate.h"
#include "csvtemplatefactory.h"

// standard includes
#include <iostream>

// Qt includes
#include <QtCore/QTextCodec>

// KDE includes
#include <kdebug.h>
#include <klocale.h>

// KABC includes
#include <kabc/addressee.h>
#include <kabc/addresseelist.h>
#include <kabc/vcardconverter.h>

using namespace KABC;

///////////////////////////////////////////////////////////////////////////////

QByteArray fromUnicode(QTextCodec* codec, const QString& text)
{
    if (codec == 0) return QByteArray();

    return codec->fromUnicode(text);
}

///////////////////////////////////////////////////////////////////////////////

QString UIDOutput::description() const
{
    return i18n("Writes the unique KABC contact identifier");
}

///////////////////////////////////////////////////////////////////////////////

bool UIDOutput::setOptions(const QByteArray& options)
{
    Q_UNUSED(options)
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool UIDOutput::setCodec(QTextCodec* codec)
{
    if (codec == 0) return false;

    m_codec = codec;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool UIDOutput::writeAddressee(const KABC::Addressee& addressee, std::ostream& stream)
{
    if (stream.bad()) return false;

    stream << fromUnicode(m_codec, addressee.uid()).constData();

    return !stream.bad();
}

///////////////////////////////////////////////////////////////////////////////

bool UIDOutput::writeAddresseeList(const KABC::AddresseeList& addresseeList,
                                  std::ostream& stream)
{
    if (stream.bad()) return false;

    AddresseeList::const_iterator it    = addresseeList.constBegin();
    AddresseeList::const_iterator endIt = addresseeList.constEnd();
    for (; it != endIt; ++it)
    {
        if (!writeAddressee(*it, stream)) return false;

        stream << std::endl;
    }

    return !stream.bad();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

VCardOutput::VCardOutput() : m_converter(0), m_vCardVersion(VCardConverter::v3_0)
{
    m_converter = new VCardConverter();
}

///////////////////////////////////////////////////////////////////////////////

VCardOutput::~VCardOutput()
{
    delete m_converter;
}

///////////////////////////////////////////////////////////////////////////////

QString VCardOutput::description() const
{
    return i18n("Exports to vCard format");
}

///////////////////////////////////////////////////////////////////////////////

bool VCardOutput::setOptions(const QByteArray& options)
{
    if (options == "v2.1")
        m_vCardVersion = VCardConverter::v2_1;
    else
        return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

QString VCardOutput::optionUsage() const
{
    QString usage =
        i18n("Optionally use a different vCard version (default is %1)",
             QString::fromLatin1("3.0"));

    usage += '\n';

    usage += "v2.1\t";
    usage += i18n("Uses the vCard version 2.1");

    return usage;
}

///////////////////////////////////////////////////////////////////////////////

bool VCardOutput::setCodec(QTextCodec* codec)
{
    if (codec == 0) return false;

    m_codec = codec;

    QString codecName = QString::fromLatin1(m_codec->name());

    if (codecName != QString::fromUtf8("UTF-8"))
    {
        QString warning = i18n("Warning: using codec '%1' with output format vcard, "
                               "but vCards are usually expected to be in UTF-8.",
                               codecName);

        std::cerr << warning.toLocal8Bit().data() << std::endl;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool VCardOutput::writeAddressee(const KABC::Addressee& addressee, std::ostream& stream)
{
    if (stream.bad()) return false;

    VCardConverter::Version version = VCardConverter::v3_0;
    switch (m_vCardVersion)
    {
        case VCardConverter::v2_1:
            version = VCardConverter::v2_1;
            break;

        case VCardConverter::v3_0:
            // for completeness, in case the enum gets extended and different
            // default value is used
            version = VCardConverter::v3_0;
            break;

        default:
            kDebug() <<"Unknown vCard version" << m_vCardVersion;
            break;
    }

    QByteArray vcard = m_converter->createVCard(addressee, version);

    // vcard is in UTF-8, only need conversion if output codec is different
    if (m_codec == QTextCodec::codecForName("UTF-8")) {
      vcard = fromUnicode(m_codec, QString::fromUtf8(vcard));
    }

    stream << vcard.constData();

    return !stream.bad();
}

///////////////////////////////////////////////////////////////////////////////

bool VCardOutput::writeAddresseeList(const KABC::AddresseeList& addresseeList,
                                     std::ostream& stream)
{
    if (stream.bad()) return false;

    VCardConverter::Version version = VCardConverter::v3_0;
    switch (m_vCardVersion)
    {
        case VCardConverter::v2_1:
            version = VCardConverter::v2_1;
            break;

        case VCardConverter::v3_0:
            // for completeness, in case the enum gets extended and different
            // default value is used
            version = VCardConverter::v3_0;
            break;

        default:
            kDebug() <<"Unknown vCard version" << m_vCardVersion;
            break;
    }

    QByteArray vcards = m_converter->createVCards(addresseeList, version);

    // vcards is in UTF-8, only need conversion if output codec is different
    if (m_codec == QTextCodec::codecForName("UTF-8")) {
      vcards = fromUnicode(m_codec, QString::fromUtf8(vcards));
    }

    stream << vcards.constData();

    return !stream.bad();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

EmailOutput::EmailOutput() : m_allEmails(false), m_includeName(false)
{
}

///////////////////////////////////////////////////////////////////////////////

QString EmailOutput::description() const
{
    return i18n("Writes email address or formatted name &lt;email address&gt;");
}

///////////////////////////////////////////////////////////////////////////////

bool EmailOutput::setOptions(const QByteArray& options)
{
    QStringList optionList = QString::fromLocal8Bit(options).split(',', QString::SkipEmptyParts);

    QStringList::const_iterator it    = optionList.constBegin();
    QStringList::const_iterator endIt = optionList.constEnd();
    for (; it != endIt; ++it)
    {
        if ((*it) == QString::fromUtf8("allemails"))
            m_allEmails = true;
        else if ((*it) == QString::fromUtf8("withname"))
            m_includeName = true;
        else
            return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

QString EmailOutput::optionUsage() const
{
    QString usage = i18n("Comma separated list of: allemails, withname");

    usage += '\n';

    usage += "allemails\t";
    usage += i18n("List all email addresses of each contact");

    usage += '\n';

    usage += "withname\t";
    usage += i18n("Prepend formatted name, e.g\n\t\tJohn Doe &lt;jdoe@foo.com&gt;");

    return usage;
}

///////////////////////////////////////////////////////////////////////////////

bool EmailOutput::setCodec(QTextCodec* codec)
{
    if (codec == 0) return false;

    m_codec = codec;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool EmailOutput::writeAddressee(const KABC::Addressee& addressee, std::ostream& stream)
{
    if (stream.bad()) return false;

    if (m_allEmails)
    {
        QStringList emails = addressee.emails();

        QStringList::const_iterator it    = emails.constBegin();
        QStringList::const_iterator endIt = emails.constEnd();

        if (it != endIt)
        {
            if (!(*it).isEmpty())
            {
                stream << fromUnicode(m_codec, decorateEmail(addressee, *it)).constData();
                if (stream.bad()) return false;
            }

            for(++it; it != endIt; ++it)
            {
                if ((*it).isEmpty()) continue;

                stream << std::endl
                       << fromUnicode(m_codec, decorateEmail(addressee, *it)).constData();

                if (stream.bad()) return false;
            }
        }
    }
    else
    {
        if (!addressee.preferredEmail().isEmpty())
        {
            stream << fromUnicode(m_codec, decorateEmail(addressee, addressee.preferredEmail())).constData();
        }
    }

    return !stream.bad();
}

///////////////////////////////////////////////////////////////////////////////

bool EmailOutput::writeAddresseeList(const KABC::AddresseeList& addresseeList,
                                     std::ostream& stream)
{
    if (stream.bad()) return false;

    AddresseeList::const_iterator it    = addresseeList.constBegin();
    AddresseeList::const_iterator endIt = addresseeList.constEnd();
    for (; it != endIt; ++it)
    {
        if ((*it).emails().count() == 0) continue;

        if (!writeAddressee(*it, stream)) return false;

        stream << std::endl;
    }

    return !stream.bad();
}

///////////////////////////////////////////////////////////////////////////////

QString EmailOutput::decorateEmail(const KABC::Addressee& addressee, const QString& email) const
{
    if (m_includeName)
        return addressee.fullEmail(email);
    else
        return email;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

MuttOutput::MuttOutput()
    : m_allEmails(false), m_queryFormat(false), m_altKeyFormat(false),
      m_preferNickNameKey(false), m_alsoNickNameKey(false)
{
}

///////////////////////////////////////////////////////////////////////////////

QString MuttOutput::description() const
{
    return i18n("Formats output as needed by the mail client mutt");
}

///////////////////////////////////////////////////////////////////////////////

bool MuttOutput::setOptions(const QByteArray& options)
{
    QStringList optionList = QString::fromLocal8Bit(options).split(',', QString::SkipEmptyParts);

    QStringList::const_iterator it    = optionList.constBegin();
    QStringList::const_iterator endIt = optionList.constEnd();
    for (; it != endIt; ++it)
    {
        if ((*it) == "allemails")
            m_allEmails = true;
        else if ((*it) == "query")
            m_queryFormat = true;
        else if ((*it) == "alias")
            m_queryFormat = false;
        else if ((*it) == "altkeys")
            m_altKeyFormat = true;
        else if ((*it) == "prefernick")
            m_preferNickNameKey = true;
        else if ((*it) == "alsonick")
            m_alsoNickNameKey = true;
        else
            return false;
    }

    if (m_alsoNickNameKey && m_preferNickNameKey)
    {
        kDebug() << "Both 'prefernick' and 'alsonick' specified, using only 'alsonick'";
        m_preferNickNameKey = false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

QString MuttOutput::optionUsage() const
{
    QString usage =
        i18n("Comma separated list of: allemails, query, alias, altkeys, prefernick, alsonick. "
             "Default is alias");

    usage += '\n';

    usage += "allemails\t";
    usage += i18n("List all email addresses of each contact");

    usage += '\n';

    usage += "query\t\t";
    usage += i18n("Use mutt's query format, e.g.\n\t\t"
                  "jdoe@foo.com [tab] John Doe\n\t\t"
                  "Conflicts with alias");

    usage += '\n';

    usage += "alias\t\t";
    usage += i18n("Use mutt's alias format, e.g.\n\t\t"
                  "alias JohDoe[tab]John Doe &lt;jdoe@foo.com&gt;\n\t\t"
                  "Conflicts with query");

    usage += '\n';

    usage += "altkeys\t\t";
    usage += i18n("Use alternative keys with alias format, e.g.\n\t\t"
                  "alias jdoe[tab]John Doe &lt;jdoe@foo.com&gt;");

    usage += '\n';

    usage += "prefernick\t";
    usage += i18n("If a nick name exists use it instead of the key, e.g.\n\t\t"
                  "alias johnny[tab]John Doe &lt;jdoe@foo.com&gt;");

    usage += '\n';

    usage += "alsonick\t";
    usage += i18n("Generate additional aliases with the nick name as the key, e.g.\n\t\t"
                  "alias johnny[tab]John Doe &lt;jdoe@foo.com&gt;\n\t\t"
                  "Deactivates 'prefernick' to avoid duplicates");

    return usage;
}

///////////////////////////////////////////////////////////////////////////////

bool MuttOutput::setCodec(QTextCodec* codec)
{
    if (codec == 0) return false;

    m_codec = codec;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool MuttOutput::writeAddressee(const KABC::Addressee& addressee, std::ostream& stream)
{
    if (stream.bad()) return false;

    const QString nickKeyString = nickNameKey(addressee);

    // if we have a key based on nick name and we prefer it over normal key use it
    // other wise use normal key
    const QString keyString =
        (m_preferNickNameKey && !nickKeyString.isEmpty()) ? nickKeyString : key(addressee);
    
    if (m_allEmails)
    {
        QStringList emails = addressee.emails();

        QStringList::const_iterator it    = emails.constBegin();
        QStringList::const_iterator endIt = emails.constEnd();

        if (it != endIt)
        {
            if (!(*it).isEmpty())
            {
                if (m_queryFormat)
                {
                    stream << fromUnicode(m_codec, *it).constData() << "\t"
                           << fromUnicode(m_codec, addressee.givenName()).constData() << " "
                           << fromUnicode(m_codec, addressee.familyName()).constData();
                }
                else
                {
                    stream << "alias " << fromUnicode(m_codec, keyString).constData() << "\t";
                    stream << fromUnicode(m_codec, addressee.givenName()).constData() << " "
                           << fromUnicode(m_codec, addressee.familyName()).constData()<< " <"
                           << fromUnicode(m_codec, *it).constData()                   << ">";
                           
                    if (m_alsoNickNameKey && !nickKeyString.isEmpty())
                    {
                        stream << std::endl;
                        stream << "alias "
                               << fromUnicode(m_codec, nickKeyString).constData()         << "\t";
                        stream << fromUnicode(m_codec, addressee.givenName()).constData() << " "
                               << fromUnicode(m_codec, addressee.familyName()).constData()<< " <"
                               << fromUnicode(m_codec, *it).constData()                   << ">";
                    }
                }

                if (stream.bad()) return false;
            }

            uint count = 1;
            for(++it; it != endIt; ++it, ++count)
            {
                if ((*it).isEmpty()) continue;

                if (m_queryFormat && count == 1)
                {
                    stream << "\t" << fromUnicode(m_codec, i18n("preferred")).constData();
                }

                stream << std::endl;
                if (m_queryFormat)
                {
                    stream << fromUnicode(m_codec, *it).constData() << "\t"
                           << fromUnicode(m_codec,  addressee.givenName()).constData() << " "
                           << fromUnicode(m_codec, addressee.familyName()).constData() << "\t"
                           << "#" << count;
                }
                else
                {
                    stream << "alias " << fromUnicode(m_codec, keyString).constData()
                           << count << "\t";
                    stream << fromUnicode(m_codec, addressee.givenName()).constData()  << " "
                           << fromUnicode(m_codec, addressee.familyName()).constData() << " <"
                           << fromUnicode(m_codec, *it).constData()                    << ">";
                           
                    if (m_alsoNickNameKey && !nickKeyString.isEmpty())
                    {
                        stream << std::endl;
                        stream << "alias "
                               << fromUnicode(m_codec, nickKeyString).constData()         << "\t";
                        stream << fromUnicode(m_codec, addressee.givenName()).constData() << " "
                               << fromUnicode(m_codec, addressee.familyName()).constData()<< " <"
                               << fromUnicode(m_codec, *it).constData()                   << ">";
                    }
                }

                if (stream.bad()) return false;
            }
        }
    }
    else
    {
        const QString preferredEmail = addressee.preferredEmail();
        if (!preferredEmail.isEmpty())
        {
            if (m_queryFormat)
            {
                stream << fromUnicode(m_codec, preferredEmail).constData()        << "\t"
                       << fromUnicode(m_codec, addressee.givenName()).constData() << " "
                       << fromUnicode(m_codec, addressee.familyName()).constData();
            }
            else
            {
                stream << "alias " << fromUnicode(m_codec, keyString).constData()  << "\t";
                stream << fromUnicode(m_codec, addressee.givenName()).constData()  << " "
                       << fromUnicode(m_codec, addressee.familyName()).constData() << " <"
                       << fromUnicode(m_codec, preferredEmail).constData()         << ">";

                if (m_alsoNickNameKey && !nickKeyString.isEmpty())
                {
                    stream << std::endl;
                    stream << "alias "
                           << fromUnicode(m_codec, nickKeyString).constData()         << "\t";
                    stream << fromUnicode(m_codec, addressee.givenName()).constData() << " "
                           << fromUnicode(m_codec, addressee.familyName()).constData()<< " <"
                           << fromUnicode(m_codec, preferredEmail).constData()        << ">";
                }
            }
        }
    }

    return !stream.bad();
}

///////////////////////////////////////////////////////////////////////////////

bool MuttOutput::writeAddresseeList(const KABC::AddresseeList& addresseeList,
                                     std::ostream& stream)
{
    if (stream.bad()) return false;

    AddresseeList::const_iterator it    = addresseeList.constBegin();
    AddresseeList::const_iterator endIt = addresseeList.constEnd();
    for (; it != endIt; ++it)
    {
        if ((*it).emails().count() == 0) continue;

        if (!writeAddressee(*it, stream)) return false;

        stream << std::endl;
    }

    return !stream.bad();
}

///////////////////////////////////////////////////////////////////////////////

QString MuttOutput::key(const KABC::Addressee& addressee) const
{
    if (m_altKeyFormat)
    {
        const QChar space = ' ';
        const QChar underscore = '_';

        if (addressee.familyName().isEmpty())
            return addressee.givenName().toLower().replace(space, underscore);
        else
            return addressee.givenName().left(1).toLower() +
                   addressee.familyName().toLower().replace(space, underscore);
    }
    else
        return addressee.givenName().left(3) + addressee.familyName().left(3);
}

QString MuttOutput::nickNameKey(const KABC::Addressee& addressee) const
{
    if (!addressee.nickName().isEmpty())
    {
        const QChar space = ' ';
        const QChar underscore = '_';
        return addressee.nickName().toLower().replace(space, underscore);
    }
    else
        return QString();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CSVOutput::CSVOutput(CSVTemplateFactory* templateFactory)
    : m_codec(0), m_template(0), m_templateFactory(templateFactory)
{
}

///////////////////////////////////////////////////////////////////////////////

CSVOutput::~CSVOutput()
{
}

///////////////////////////////////////////////////////////////////////////////

QString CSVOutput::description() const
{
    return i18n("Writes the data as a delimiter separated list of values");
}

///////////////////////////////////////////////////////////////////////////////

bool CSVOutput::setOptions(const QByteArray& options)
{
    QString name = QString::fromLocal8Bit(options);
    if (name.isEmpty()) return false;

    m_template = m_templateFactory->createCachedTemplate(name);

    return m_template != 0;
}

///////////////////////////////////////////////////////////////////////////////

QString CSVOutput::optionUsage() const
{
    QString usage =
        i18n("Specify one of the following CSV templates:");

    usage += '\n';

    const QMap<QString, QString> templateNames = m_templateFactory->templateNames();

    QMap<QString, QString>::const_iterator it    = templateNames.constBegin();
    QMap<QString, QString>::const_iterator endIt = templateNames.constEnd();
    for (; it != endIt; ++it)
    {
        QString name = it.key();
        QString templateName = it.value();

        usage += name;

        usage += name.length() < 8 ? "\t\t" : "\t";

        usage += templateName;

        usage += '\n';
    }

    return usage;
}

///////////////////////////////////////////////////////////////////////////////

bool CSVOutput::setCodec(QTextCodec* codec)
{
    if (codec == 0) return false;

    m_codec = codec;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool CSVOutput::writeAddressee(const KABC::Addressee& addressee, std::ostream& stream)
{
    if (stream.bad()) return false;

    if (m_template == 0) m_template = CSVTemplate::defaultTemplate();

    QStringList columns;
    for (int i = 0; i < m_template->columns(); ++i)
    {
        QString text = m_template->fieldText(i, addressee);
        text.replace(QChar('\n'), "\\n");

        columns.append(m_template->quote() + text + m_template->quote());
    }

    stream << fromUnicode(m_codec, columns.join(m_template->delimiter())).constData();

    return !stream.bad();
}

///////////////////////////////////////////////////////////////////////////////

bool CSVOutput::writeAddresseeList(const KABC::AddresseeList& addresseeList,
                                  std::ostream& stream)
{
    if (stream.bad()) return false;

    AddresseeList::const_iterator it    = addresseeList.constBegin();
    AddresseeList::const_iterator endIt = addresseeList.constEnd();
    for (; it != endIt; ++it)
    {
        if (!writeAddressee(*it, stream)) return false;

        stream << std::endl;
    }

    return !stream.bad();
}

// End of file

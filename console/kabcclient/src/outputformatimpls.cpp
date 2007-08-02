//
//  Copyright (C) 2005 - 2006 Kevin Krammer <kevin.krammer@gmx.at>
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

// standard includes
#include <iostream>

// Qt includes
#include <QTextCodec>

// KDE includes
#include <kdebug.h>
#include <klocale.h>

// KABC includes
#include <kabc/addressee.h>
#include <kabc/addresseelist.h>
#include <kabc/vcardconverter.h>

// local includes
#include "csvtemplate.h"
#include "csvtemplatefactory.h"
#include "outputformatimpls.h"

using namespace KABC;

///////////////////////////////////////////////////////////////////////////////

static const char* fromUnicode(QTextCodec* codec, const QString& text)
{
    if (codec == 0) return "";

    return codec->fromUnicode(text).data();
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

    stream << fromUnicode(m_codec, addressee.uid());

    return !stream.bad();
}

///////////////////////////////////////////////////////////////////////////////

bool UIDOutput::writeAddresseeList(const KABC::AddresseeList& addresseeList,
                                  std::ostream& stream)
{
    if (stream.bad()) return false;

    AddresseeList::const_iterator it    = addresseeList.begin();
    AddresseeList::const_iterator endIt = addresseeList.end();
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
    return i18n("Exports to VCard format");
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
             QString::fromAscii("3.0"));

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

    QString codecName = QString::fromAscii(m_codec->name());

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

    QString vcard = m_converter->createVCard(addressee, version);
    stream << fromUnicode(m_codec, vcard);

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

    QString vcards = m_converter->createVCards(addresseeList, version);
    stream << fromUnicode(m_codec, vcards);

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
    return i18n("Writes email address or formatted name <email address>");
}

///////////////////////////////////////////////////////////////////////////////

bool EmailOutput::setOptions(const QByteArray& options)
{
    QStringList optionList = QString::fromLocal8Bit(options).split(',', QString::SkipEmptyParts);

    QStringList::const_iterator it    = optionList.begin();
    QStringList::const_iterator endIt = optionList.end();
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
    usage += i18n("Prepend formatted name, e.g\n\t\tJohn Doe <jdoe@foo.com>");

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

        QStringList::const_iterator it    = emails.begin();
        QStringList::const_iterator endIt = emails.end();

        if (it != endIt)
        {
            if (!(*it).isEmpty())
            {
                stream << fromUnicode(m_codec, decorateEmail(addressee, *it));
                if (stream.bad()) return false;
            }

            for(++it; it != endIt; ++it)
            {
                if ((*it).isEmpty()) continue;

                stream << std::endl
                       << fromUnicode(m_codec, decorateEmail(addressee, *it));

                if (stream.bad()) return false;
            }
        }
    }
    else
    {
        if (!addressee.preferredEmail().isEmpty())
        {
            stream << fromUnicode(m_codec, decorateEmail(addressee, addressee.preferredEmail()));
        }
    }

    return !stream.bad();
}

///////////////////////////////////////////////////////////////////////////////

bool EmailOutput::writeAddresseeList(const KABC::AddresseeList& addresseeList,
                                     std::ostream& stream)
{
    if (stream.bad()) return false;

    AddresseeList::const_iterator it    = addresseeList.begin();
    AddresseeList::const_iterator endIt = addresseeList.end();
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

MuttOutput::MuttOutput() : m_allEmails(false), m_queryFormat(false), m_altKeyFormat(false)
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

    QStringList::const_iterator it    = optionList.begin();
    QStringList::const_iterator endIt = optionList.end();
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
        else
            return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

QString MuttOutput::optionUsage() const
{
    QString usage =
        i18n("Comma separated list of: allemails, query, alias, altkeys. Default is alias");

    usage += '\n';

    usage += "allemails\t";
    usage += i18n("List all email addresses of each contact");

    usage += '\n';

    usage += "query\t\t";
    usage += i18n("Use mutt's query format, e.g.\n\t\t"
                  "jdoe@foo.com <tab> John Doe\n\t\t"
                  "Conflicts with alias");

    usage += '\n';

    usage += "alias\t\t";
    usage += i18n("Use mutt's alias format, e.g.\n\t\t"
                  "alias JohDoe<tab>John Doe <jdoe@foo.com>\n\t\t"
                  "Conflicts with query");

    usage += '\n';

    usage += "altkeys\t\t";
    usage += i18n("Use alternative keys with alias format, e.g.\n\t\t"
                  "alias jdoe<tab>John Doe <jdoe@foo.com>");

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

    if (m_allEmails)
    {
        QStringList emails = addressee.emails();

        QStringList::const_iterator it    = emails.begin();
        QStringList::const_iterator endIt = emails.end();

        if (it != endIt)
        {
            const QString keyString = key(addressee);

            if (!(*it).isEmpty())
            {
                if (m_queryFormat)
                {
                    stream << fromUnicode(m_codec, *it) << "\t"
                           << fromUnicode(m_codec, addressee.givenName()) << " "
                           << fromUnicode(m_codec, addressee.familyName());
                }
                else
                {
                    stream << "alias " << fromUnicode(m_codec, keyString) << "\t";
                    stream << fromUnicode(m_codec, addressee.givenName()) << " "
                           << fromUnicode(m_codec, addressee.familyName())<< " <"
                           << fromUnicode(m_codec, *it)                   << ">";
                }

                if (stream.bad()) return false;
            }

            uint count = 1;
            for(++it; it != endIt; ++it, ++count)
            {
                if ((*it).isEmpty()) continue;

                if (m_queryFormat && count == 1)
                {
                    stream << "\t" << fromUnicode(m_codec, i18n("preferred"));
                }

                stream << std::endl;
                if (m_queryFormat)
                {
                    stream << fromUnicode(m_codec, *it) << "\t"
                           << fromUnicode(m_codec,  addressee.givenName()) << " "
                           << fromUnicode(m_codec, addressee.familyName()) << "\t"
                           << "#" << count;
                }
                else
                {
                    stream << "alias " << fromUnicode(m_codec, keyString)
                           << count << "\t";
                    stream << fromUnicode(m_codec, addressee.givenName())  << " "
                           << fromUnicode(m_codec, addressee.familyName()) << " <"
                           << fromUnicode(m_codec, *it)                    << ">";
                }

                if (stream.bad()) return false;
            }
        }
    }
    else
    {
        if (!addressee.preferredEmail().isEmpty())
        {
            if (m_queryFormat)
            {
                stream << fromUnicode(m_codec, addressee.preferredEmail()) << "\t"
                       << fromUnicode(m_codec, addressee.givenName())      << " "
                       << fromUnicode(m_codec, addressee.familyName());
            }
            else
            {
                stream << "alias " << fromUnicode(m_codec, key(addressee)) << "\t";
                stream << fromUnicode(m_codec, addressee.givenName())      << " "
                       << fromUnicode(m_codec, addressee.familyName())     << " <"
                       << fromUnicode(m_codec, addressee.preferredEmail()) << ">";
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

    AddresseeList::const_iterator it    = addresseeList.begin();
    AddresseeList::const_iterator endIt = addresseeList.end();
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

    QMap<QString, QString>::const_iterator it    = templateNames.begin();
    QMap<QString, QString>::const_iterator endIt = templateNames.end();
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
        text.replace("\n", "\\n");

        columns.append(m_template->quote() + text + m_template->quote());
    }

    stream << fromUnicode(m_codec, columns.join(m_template->delimiter()));

    return !stream.bad();
}

///////////////////////////////////////////////////////////////////////////////

bool CSVOutput::writeAddresseeList(const KABC::AddresseeList& addresseeList,
                                  std::ostream& stream)
{
    if (stream.bad()) return false;

    AddresseeList::const_iterator it    = addresseeList.begin();
    AddresseeList::const_iterator endIt = addresseeList.end();
    for (; it != endIt; ++it)
    {
        if (!writeAddressee(*it, stream)) return false;

        stream << std::endl;
    }

    return !stream.bad();
}

// End of file

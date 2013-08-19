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

// local includes
#include "inputformatimpls.h"
#include "csvtemplate.h"
#include "csvtemplatefactory.h"

// standard includes
#include <iostream>
#include <string>

// Qt includes
#include <QtCore/QTextCodec>

// KDE includes
#include <klocale.h>

// KABC includes
#include <kabc/addressee.h>
#include <kabc/addresseedialog.h>
#include <kabc/addresseelist.h>
#include <kabc/vcardconverter.h>

using namespace KABC;

///////////////////////////////////////////////////////////////////////////////

QString UIDInput::description() const
{
    return i18n("Interprets input as a unique KABC contact identifier");
}

///////////////////////////////////////////////////////////////////////////////

bool UIDInput::setOptions(const QByteArray& options)
{
    Q_UNUSED(options)
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool UIDInput::setCodec(QTextCodec* codec)
{
    if (codec == 0) return false;

    m_codec = codec;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

KABC::Addressee UIDInput::readAddressee(std::istream& stream)
{
    if (stream.bad()) return KABC::Addressee();

    KABC::Addressee addressee;

    std::string line;
    getline(stream, line);
    while (line.empty() && !stream.eof())
    {
        getline(stream, line);
    }

    if (stream.eof()) return addressee;

    QString uid = m_codec->toUnicode(line.c_str(), line.size());

    addressee.setUid(uid);

    return addressee;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

VCardInput::VCardInput() : m_converter(0)
{
    m_converter = new VCardConverter();
}

///////////////////////////////////////////////////////////////////////////////

QString VCardInput::description() const
{
    return i18n("Interprets input as vCard data");
}

///////////////////////////////////////////////////////////////////////////////

VCardInput::~VCardInput()
{
    delete m_converter;
}

///////////////////////////////////////////////////////////////////////////////

bool VCardInput::setOptions(const QByteArray& options)
{
    Q_UNUSED(options)
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool VCardInput::setCodec(QTextCodec* codec)
{
    if (codec == 0) return false;

    m_codec = codec;

    QString codecName = QString::fromLatin1(m_codec->name());

    if (codecName != QString::fromUtf8("UTF-8"))
    {
        QString warning = i18n("Warning: using codec '%1' with input format vcard, "
                               "but vCards are usually expected to be in UTF-8.",
                               codecName);

        std::cerr << warning.toLocal8Bit().data() << std::endl;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

KABC::Addressee VCardInput::readAddressee(std::istream& stream)
{
    if (stream.bad()) return KABC::Addressee();

    std::string line;
    // read any empty lines
    while (line.empty() && !stream.eof())
    {
        getline(stream, line);
    }
    if (line.empty()) return KABC::Addressee();

    QString input = m_codec->toUnicode(line.c_str(), line.size());
    input += '\n';

    QString inputLine;
    while (!inputLine.isEmpty() || !stream.eof())
    {
        getline(stream, line);
        inputLine = m_codec->toUnicode(line.c_str(), line.size());

        input += inputLine;
        input += '\n';
        if (inputLine.startsWith("END:VCARD")) break;
    }

    return m_converter->parseVCard(input.toUtf8());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

QString EmailInput::description() const
{
    return i18n("Interprets input as email and optional name");
}

///////////////////////////////////////////////////////////////////////////////

bool EmailInput::setOptions(const QByteArray& options)
{
    Q_UNUSED(options)
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool EmailInput::setCodec(QTextCodec* codec)
{
    if (codec == 0) return false;

    m_codec = codec;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

KABC::Addressee EmailInput::readAddressee(std::istream& stream)
{
    if (stream.bad()) return KABC::Addressee();

    KABC::Addressee addressee;

    std::string line;
    getline(stream, line);
    while (line.empty() && !stream.eof())
    {
        getline(stream, line);
    }

    if (stream.eof()) return addressee;

    QString rawEmail = m_codec->toUnicode(line.c_str(), line.size());
    QString name;
    QString email;
    KABC::Addressee::parseEmailAddress(rawEmail, name, email);

    if (!email.isEmpty() && email.indexOf("@") != -1)
    {
        addressee.insertEmail(email, true);

        if (!name.isEmpty())
        {
            addressee.setNameFromString(name);
        }

        addressee.setUid(QString());
    }

    return addressee;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

QString SearchInput::description() const
{
    return i18n("Tries to get email and name from input,\n\t\t"
                "otherwise sets input text for both");
}

///////////////////////////////////////////////////////////////////////////////

bool SearchInput::setOptions(const QByteArray& options)
{
    Q_UNUSED(options)
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool SearchInput::setCodec(QTextCodec* codec)
{
    if (codec == 0) return false;

    m_codec = codec;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

KABC::Addressee SearchInput::readAddressee(std::istream& stream)
{
    if (stream.bad()) return KABC::Addressee();

    KABC::Addressee addressee;

    std::string line;
    getline(stream, line);
    while (line.empty() && !stream.eof())
    {
        getline(stream, line);
    }

    if (stream.eof()) return addressee;

    QString rawEmail = m_codec->toUnicode(line.c_str(), line.size());
    QString name;
    QString email;
    KABC::Addressee::parseEmailAddress(rawEmail, name, email);

    if (email.isEmpty() || email.indexOf("@") == -1)
    {
        addressee.insertEmail(rawEmail, true);
    }
    else
    {
        addressee.insertEmail(email, true);
    }

    if (!name.isEmpty())
    {
        addressee.setNameFromString(name);
    }
    else
    {
        addressee.setNameFromString(rawEmail);
    }

    addressee.setUid(QString());

    return addressee;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

QString NameInput::description() const
{
    return i18n("Interprets the input as a name.\n\t\t"
                "Recommended format is 'lastname, firstname middlename'");
}

///////////////////////////////////////////////////////////////////////////////

bool NameInput::setOptions(const QByteArray& options)
{
    Q_UNUSED(options)
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool NameInput::setCodec(QTextCodec* codec)
{
    if (codec == 0) return false;

    m_codec = codec;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

KABC::Addressee NameInput::readAddressee(std::istream& stream)
{
    if (stream.bad()) return KABC::Addressee();

    KABC::Addressee addressee;

    std::string line;
    getline(stream, line);
    while (line.empty() && !stream.eof())
    {
        getline(stream, line);
    }

    if (stream.eof()) return addressee;

    QString name = m_codec->toUnicode(line.c_str(), line.size());

    if (!name.isEmpty())
    {
        addressee.setNameFromString(name);
    }

    addressee.setUid(QString());

    return addressee;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CSVInput::CSVInput(CSVTemplateFactory* templateFactory)
    : m_codec(0), m_template(0), m_templateFactory(templateFactory)
{
}

///////////////////////////////////////////////////////////////////////////////

QString CSVInput::description() const
{
    return i18n("Interprets the input as a delimiter separated list of fields.");
}

///////////////////////////////////////////////////////////////////////////////

bool CSVInput::setOptions(const QByteArray& options)
{
    QString name = QString::fromLocal8Bit(options);
    if (name.isEmpty()) return false;

    m_template = m_templateFactory->createCachedTemplate(name);

    return m_template != 0;
}

///////////////////////////////////////////////////////////////////////////////

QString CSVInput::optionUsage() const
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

bool CSVInput::setCodec(QTextCodec* codec)
{
    if (codec == 0) return false;

    m_codec = codec;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

KABC::Addressee CSVInput::readAddressee(std::istream& stream)
{
    if (stream.bad()) return KABC::Addressee();

    if (m_template == 0) m_template = CSVTemplate::defaultTemplate();

    KABC::Addressee addressee;

    std::string line;
    getline(stream, line);
    while (line.empty() && !stream.eof())
    {
        getline(stream, line);
    }

    if (stream.eof()) return addressee;

    QString values = m_codec->toUnicode(line.c_str(), line.size());

    if (!values.isEmpty())
    {
        QStringList list = split(values);

        QStringList::const_iterator it    = list.constBegin();
        QStringList::const_iterator endIt = list.constEnd();
        for (int i = 0; it != endIt; ++it, ++i)
        {
            m_template->setFieldText(i, addressee, *it);
        }
    }

    return addressee;
}

///////////////////////////////////////////////////////////////////////////////

QStringList CSVInput::split(const QString& values) const
{
    const QString quote     = m_template->quote();
    const QString delimiter = m_template->delimiter();

    if (quote.isEmpty()) return values.split(delimiter, QString::KeepEmptyParts);

    QString remaining = values;

    QStringList list;
    bool quoted = false;
    while (!remaining.isEmpty())
    {
        if (quoted)
        {
            int quoteIndex = remaining.indexOf(quote);
            if (quoteIndex >= 0)
            {
                list.append(remaining.left(quoteIndex));
                remaining = remaining.mid(quoteIndex + quote.length());

                if (remaining.indexOf(delimiter) == 0)
                {
                    remaining = remaining.mid(delimiter.length());
                }
            }
            else
            {
                list.append(remaining);
                remaining.clear();
            }

            quoted = false;
        }
        else
        {
            int quoteIndex = remaining.indexOf(quote);

            if (quoteIndex == 0)
            {
                quoted = true;

                remaining = remaining.mid(quote.length());
            }
            else
            {
                int delimiterIndex = remaining.indexOf(delimiter);
                if (delimiterIndex >= 0)
                {
                    list.append(remaining.left(delimiterIndex));
                    remaining = remaining.mid(delimiterIndex + delimiter.length());
                }
                else
                {
                    list.append(remaining);
                    remaining.clear();
                }
            }
        }
    }

    return list;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class DialogInputPrivate
{
public:
    DialogInputPrivate() : showDialog(true) {}

public:
    bool showDialog;
    KABC::AddresseeList addresseeList;
};

///////////////////////////////////////////////////////////////////////////////

DialogInput::DialogInput() : m_private(new DialogInputPrivate())
{
}

///////////////////////////////////////////////////////////////////////////////

DialogInput::~DialogInput()
{
    delete m_private;
}

///////////////////////////////////////////////////////////////////////////////

QString DialogInput::description() const
{
    return i18n("Select contacts in a dialog instead of reading input text");
}

///////////////////////////////////////////////////////////////////////////////

bool DialogInput::setOptions(const QByteArray& options)
{
    Q_UNUSED(options)
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool DialogInput::setCodec(QTextCodec* codec)
{
    if (codec == 0) return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////

KABC::Addressee DialogInput::readAddressee(std::istream& stream)
{
    KABC::Addressee addressee;

    // on first call show dialog for addressee selection
    if (m_private->showDialog)
    {
        m_private->addresseeList = KABC::AddresseeDialog::getAddressees(0);
        m_private->showDialog = false;
    }

    // As long as there are addressees left in the selection list take the first
    // one and use its UID as the output of this InputFormat.
    // When all selected addressees have been processed, mark stream as empty
    if (!m_private->addresseeList.isEmpty())
    {
        KABC::Addressee nextAddressee = m_private->addresseeList.front();

        // using the UID avoid multiple matches by name or email of the addressee
        if (!nextAddressee.isEmpty()) addressee.setUid(nextAddressee.uid());

        m_private->addresseeList.pop_front();
    }
    else
        stream.setstate(std::ios_base::eofbit);

    return addressee;
}

// End of file

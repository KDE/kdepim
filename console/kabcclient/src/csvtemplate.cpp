//
//  Copyright (C) 2005 Kevin Krammer <kevin.krammer@gmx.at>
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

// Qt includes
#include <qdatetime.h>

// KDE includes
#include <kconfigbase.h>

// KABC includes
#include <kabc/address.h>
#include <kabc/addressee.h>

// local includes
#include "csvtemplate.h"

using namespace KABC;

///////////////////////////////////////////////////////////////////////////////

CSVTemplate* CSVTemplate::m_defaultTemplate = 0;

///////////////////////////////////////////////////////////////////////////////

CSVTemplate::CSVTemplate(KConfigBase* config) : m_columns(0)
{
    if (config == 0) return;

    QMap<QString, QString> columnMap = config->entryMap("csv column map");
    
    QMap<QString, QString>::const_iterator it    = columnMap.begin();
    QMap<QString, QString>::const_iterator endIt = columnMap.end();
    for (; it != endIt; ++it)
    {
        if (it.key().isEmpty() || it.data().isEmpty()) continue;

        bool ok = false;
        int column = it.key().toInt(&ok);
        if (!ok) continue;
        
        int field = it.data().toInt(&ok);
        if (!ok) continue;

        m_columnToField.insert(column, field);
    }

    KConfigGroupSaver saver(config, "General");
    KConfigBase* general = saver.config();
    
    m_datePattern = general->readEntry("DatePattern");
    if (m_datePattern.isEmpty()) m_datePattern = "Y-M-D";
    createDateFormat();
    
    m_columns = general->readNumEntry("Columns");
    if (m_columns < 0) m_columns = 0;

    switch (general->readNumEntry("DelimiterType"))
    {
        case 1:
            m_delimiter = ";";
            break;
            
        case 2:
            m_delimiter = "\t";
            break;

        case 3:
            m_delimiter = " ";
            break;

        case 4:
            m_delimiter = general->readEntry("DelimiterOther");
            break;

        default:
            m_delimiter = ",";        
    }
    if (m_delimiter.isEmpty()) m_delimiter = ",";

    switch (general->readNumEntry("QuoteType"))
    {
        case 1:
            m_quote = "'";
            break;
            
        case 2:
            break;

        default:
            m_quote = "\"";
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////

QString CSVTemplate::fieldText(int column, const KABC::Addressee& addressee) const
{
    if (column < 0 || addressee.isEmpty()) return QString::null;
    if (m_columnToField.isEmpty()) return QString::null;

    QMap<int, int>::const_iterator it = m_columnToField.find(column);
    if (it == m_columnToField.end()) return QString::null;
        
    QString text;
    switch (it.data())
    {
        case  0: // "Formatted Name"
            text = addressee.formattedName();
            break;            
        case  1: // "Family Name"
            text = addressee.familyName();
            break;
        case  2: // "Given Name"
            text = addressee.givenName();
            break;
        case  3: // "Additional Names"
            text = addressee.additionalName();
            break;
        case  4: // "Honorific Prefixes"
            text = addressee.prefix();
            break;
        case  5: // "Honorific Suffixes"
            text = addressee.suffix();
            break;
        case  6: // "Nick Name"
            text = addressee.nickName();
            break;
        case  7: // "Birthday"
            text = formatDate(addressee.birthday());
            break;
        case  8: // "Home Address Street"
            text = addressee.address(Address::Home).street();
            break;
        case  9: // "Home Address Locality"
            text = addressee.address(Address::Home).locality();
            break;
        case 10: // "Home Address Region"
            text = addressee.address(Address::Home).region();
            break;
        case 11: // "Home Address Postal Code"
            text = addressee.address(Address::Home).postalCode();
            break;
        case 12: // "Home Address Country"
            text = addressee.address(Address::Home).country();
            break;
        case 13: // "Home Address Label"
            text = addressee.address(Address::Home).label();
            break;
        case 14: // "Business Address Street"
            text = addressee.address(Address::Work).street();
            break;
        case 15: // "Business Address Locality"
            text = addressee.address(Address::Work).locality();
            break;
        case 16: // "Business Address Region"
            text = addressee.address(Address::Work).region();
            break;
        case 17: // "Business Address Postal Code"
            text = addressee.address(Address::Work).postalCode();
            break;
        case 18: // "Business Address Country"
            text = addressee.address(Address::Work).country();
            break;
        case 19: // "Business Address Label"
            text = addressee.address(Address::Work).label();
            break;
        case 20: // "Home Phone"
            text = addressee.phoneNumber(PhoneNumber::Home).number();
            break;
        case 21: // "Business Phone"
            text = addressee.phoneNumber(PhoneNumber::Work).number();
            break;
        case 22: // "Mobile Phone"
            text = addressee.phoneNumber(PhoneNumber::Cell).number();
            break;
        case 23: // "Home Fax"
            text = addressee.phoneNumber(PhoneNumber::Fax | PhoneNumber::Home).number();
            break;
        case 24: // "Business Fax"
            text = addressee.phoneNumber(PhoneNumber::Fax | PhoneNumber::Work).number();
            break;
        case 25: // "Car Phone"
            text = addressee.phoneNumber(PhoneNumber::Car).number();
            break;
        case 26: // "Isdn"
            text = addressee.phoneNumber(PhoneNumber::Isdn).number();
            break;
        case 27: // "Pager"
            text = addressee.phoneNumber(PhoneNumber::Pager).number();
            break;
        case 28: // "Email Address"
            text = addressee.preferredEmail();
            break;
        case 29: // "Mail Client"
            text = addressee.mailer();
            break;
        case 30: // "Title"
            text = addressee.title();
            break;
        case 31: // "Role"
            text = addressee.role();
            break;
        case 32: // "Organization"
            text = addressee.organization();
            break;
        case 33: // "Note"
            text = addressee.note();
            break;
        case 34: // "URL"
            text = addressee.url().prettyURL();
            break;
        case 35: // "Department"
            break;
        case 36: // "Profession"
            break;
        case 37: // "Assistant's Name"
            break;
        case 38: // "Manager's Name"
            break;
        case 39: // "Spouse's Name"
            break;
        case 40: // "Office"
            break;
        case 41: // "IM Address"
            break;
        case 42: // "Anniversary"
            break;

        default:
            break;
    }

    return text;
}

///////////////////////////////////////////////////////////////////////////////

void CSVTemplate::setFieldText(int column, KABC::Addressee& addressee, const QString& text) const
{
    if (column < 0 || text.isEmpty()) return;
    if (m_columnToField.isEmpty()) return;

    QMap<int, int>::const_iterator it = m_columnToField.find(column);
    if (it == m_columnToField.end()) return;
    
    Address address;
    PhoneNumber phone;
        
    switch (it.data())
    {
        case  0: // "Formatted Name"
            addressee.setFormattedName(text);
            break;            
        case  1: // "Family Name"
            addressee.setFamilyName(text);
            break;
        case  2: // "Given Name"
            addressee.setGivenName(text);
            break;
        case  3: // "Additional Names"
            addressee.setAdditionalName(text);
            break;
        case  4: // "Honorific Prefixes"
            addressee.setPrefix(text);
            break;
        case  5: // "Honorific Suffixes"
            addressee.setSuffix(text);
            break;
        case  6: // "Nick Name"
            addressee.setNickName(text);
            break;
        case  7: // "Birthday"
            addressee.setBirthday(parseDate(text));
            break;
        case  8: // "Home Address Street"
            address = addressee.address(Address::Home);
            address.setStreet(text);
            addressee.insertAddress(address);
            break;
        case  9: // "Home Address Locality"
            address = addressee.address(Address::Home);
            address.setLocality(text);
            addressee.insertAddress(address);
            break;
        case 10: // "Home Address Region"
            address = addressee.address(Address::Home);
            address.setRegion(text);
            addressee.insertAddress(address);
            break;
        case 11: // "Home Address Postal Code"
            address = addressee.address(Address::Home);
            address.setPostalCode(text);
            addressee.insertAddress(address);
            break;
        case 12: // "Home Address Country"
            address = addressee.address(Address::Home);
            address.setCountry(text);
            addressee.insertAddress(address);
            break;
        case 13: // "Home Address Label"
            address = addressee.address(Address::Home);
            address.setLabel(text);
            addressee.insertAddress(address);
            break;
        case 14: // "Business Address Street"
            address = addressee.address(Address::Work);
            address.setStreet(text);
            addressee.insertAddress(address);
            break;
        case 15: // "Business Address Locality"
            address = addressee.address(Address::Work);
            address.setLocality(text);
            addressee.insertAddress(address);
            break;
        case 16: // "Business Address Region"
            address = addressee.address(Address::Work);
            address.setRegion(text);
            addressee.insertAddress(address);
            break;
        case 17: // "Business Address Postal Code"
            address = addressee.address(Address::Work);
            address.setPostalCode(text);
            addressee.insertAddress(address);
            break;
        case 18: // "Business Address Country"
            address = addressee.address(Address::Work);
            address.setCountry(text);
            addressee.insertAddress(address);
            break;
        case 19: // "Business Address Label"
            address = addressee.address(Address::Work);
            address.setLabel(text);
            addressee.insertAddress(address);
            break;
        case 20: // "Home Phone"
            phone = PhoneNumber(text, PhoneNumber::Home);
            addressee.insertPhoneNumber(phone);
            break;
        case 21: // "Business Phone"
            phone = PhoneNumber(text, PhoneNumber::Work);
            addressee.insertPhoneNumber(phone);
            break;
        case 22: // "Mobile Phone"
            phone = PhoneNumber(text, PhoneNumber::Cell);
            addressee.insertPhoneNumber(phone);
            break;
        case 23: // "Home Fax"
            phone = PhoneNumber(text, PhoneNumber::Fax | PhoneNumber::Home);
            addressee.insertPhoneNumber(phone);
            break;
        case 24: // "Business Fax"
            phone = PhoneNumber(text, PhoneNumber::Fax | PhoneNumber::Work);
            addressee.insertPhoneNumber(phone);
            break;
        case 25: // "Car Phone"
            phone = PhoneNumber(text, PhoneNumber::Car);
            addressee.insertPhoneNumber(phone);
            break;
        case 26: // "Isdn"
            phone = PhoneNumber(text, PhoneNumber::Isdn);
            addressee.insertPhoneNumber(phone);
            break;
        case 27: // "Pager"
            phone = PhoneNumber(text, PhoneNumber::Pager);
            addressee.insertPhoneNumber(phone);
            break;
        case 28: // "Email Address"
            addressee.insertEmail(text);
            break;
        case 29: // "Mail Client"
            addressee.setMailer(text);
            break;
        case 30: // "Title"
            addressee.setTitle(text);
            break;
        case 31: // "Role"
            addressee.setRole(text);
            break;
        case 32: // "Organization"
            addressee.setOrganization(text);
            break;
        case 33: // "Note"
            addressee.setNote(text);
            break;
        case 34: // "URL"
            addressee.setUrl(text);
            break;
        case 35: // "Department"
            break;
        case 36: // "Profession"
            break;
        case 37: // "Assistant's Name"
            break;
        case 38: // "Manager's Name"
            break;
        case 39: // "Spouse's Name"
            break;
        case 40: // "Office"
            break;
        case 41: // "IM Address"
            break;
        case 42: // "Anniversary"
            break;

        default:
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////
    
CSVTemplate* CSVTemplate::defaultTemplate()
{
    if (m_defaultTemplate == 0)
    {
        m_defaultTemplate = new CSVTemplate("Y-M-D");

        m_defaultTemplate->m_quote     = "\"";
        m_defaultTemplate->m_delimiter = ",";
        
        m_defaultTemplate->m_columns   = 42;
        for (int i = 0; i < m_defaultTemplate->m_columns; ++i)
        {
            m_defaultTemplate->m_columnToField[i] = i + 1; 
        }
    }
    
    return m_defaultTemplate;
}

///////////////////////////////////////////////////////////////////////////////

CSVTemplate::CSVTemplate(const QString& datePattern)
{
    m_datePattern = datePattern;
    createDateFormat();
}

///////////////////////////////////////////////////////////////////////////////

QString CSVTemplate::formatDate(const QDateTime& date) const
{
    if (!date.isValid()) return QString::null;
    
    return date.toString(m_dateFormat);
}

///////////////////////////////////////////////////////////////////////////////

QDateTime CSVTemplate::parseDate(const QString& text) const
{
    if (text.isEmpty()) return QDateTime();

    int year  = 0;
    int month = 0;
    int day   = 0;

    QCString pattern = m_datePattern.ascii();
    bool ok = true;
    uint pos = 0;
    for (uint i = 0; ok && i < pattern.length(); ++i)
    {
        switch (pattern[i])
        {
            case 'Y': // four digit year
                if ((pos + 3) >= text.length())
                {
                    ok = false;
                }
                else
                {
                    year = text.mid(pos, 4).toInt(&ok);
                    pos += 4;
                }
                break;
                
            case 'y': // two digit 19xx year
                if ((pos + 1) >= text.length())
                {
                    ok = false;
                }
                else
                {
                    year = 1900 + text.mid(pos, 2).toInt(&ok);
                    pos += 2;
                }
                break;
                
            case 'M': // two digit month
                if ((pos + 1) >= text.length())
                {
                    ok = false;
                }
                else
                {
                    month = text.mid(pos, 2).toInt(&ok);
                    pos += 2;
                }
                break;
                
            case 'm': // one or two digit month
                if ((pos + 1) < text.length() && text[pos+1].isDigit())
                {
                    month = text.mid(pos, 2).toInt(&ok);
                    pos += 2;
                }
                else
                {
                    month = text.mid(pos, 1).toInt(&ok);
                    pos += 1;
                }
                break;
                
            case 'D': // two digit day
                if ((pos + 1) >= text.length())
                {
                    ok = false;
                }
                else
                {
                    day = text.mid(pos, 2).toInt(&ok);
                    pos += 2;
                }
                break;
                
            case 'd': // one or two digit day
                if ((pos + 1) < text.length() && text[pos+1].isDigit())
                {
                    day = text.mid(pos, 2).toInt(&ok);
                    pos += 2;
                }
                else
                {
                    day = text.mid(pos, 1).toInt(&ok);
                    pos += 1;
                }
                break;
                
            default:
                ok = QChar(pattern[i]) == text[pos];
                pos++;
                break;
        }
    }

    ok = ok && pos >= text.length();

    if (ok && year > 0 && month > 0 && day > 0) return QDateTime(QDate(year, month, day));
    
    return QDateTime();
}

///////////////////////////////////////////////////////////////////////////////

void CSVTemplate::createDateFormat()
{
    QCString datePattern = m_datePattern.ascii();
    
    for (uint i = 0; i < datePattern.length(); ++i)
    {
        switch (datePattern[i])
        {
            case 'Y':
                m_dateFormat.append("yyyy");
                break;
        
            case 'y':
                m_dateFormat.append("yy");
                break;
        
            case 'M':
                m_dateFormat.append("MM");
                break;
    
            case 'm':
                m_dateFormat.append("m");
                break;
                
            case 'D':
                m_dateFormat.append("dd");
                break;
                
            case 'd':
                m_dateFormat.append("d");
                break;
                
            default:
                m_dateFormat.append(datePattern[i]);
                break;
        }
    }
}

// End of file

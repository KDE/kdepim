//
//  Copyright (C) 2005 - 2011 Kevin Krammer <kevin.krammer@gmx.at>
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

#ifndef OUTPUTFORMATIMPLS_H
#define OUTPUTFORMATIMPLS_H

// local includes
#include "outputformat.h"

// forward declarations
class CSVTemplate;
class CSVTemplateFactory;
class QTextCodec;

namespace KABC
{
    class VCardConverter;
}

/**
* @brief Output formatter for KABC UIDs
*
* Writes each contact's unique identifier, one per line
*
* Available through FormatFactory::outputFormat(), name "uid"
*
* @author Kevin Krammer, <kevin.krammer@gmx.at>
*
* @see @ref formathandling
*/
class UIDOutput : public OutputFormat
{
public:
    virtual QString description() const;

    virtual bool setOptions(const QByteArray& options);

    virtual bool setCodec(QTextCodec* codec);

    virtual bool writeAddressee(const KABC::Addressee& addressee, std::ostream& stream);
    virtual bool writeAddresseeList(const KABC::AddresseeList& addresseeList,
                                    std::ostream& stream);

private:
    /**
    * The codec to use for converting QString's UTF encoding to output text
    *
    * @see QTextCodec::fromUnicode()
    */
    QTextCodec* m_codec;
};

/**
* @brief Output formatter for VCard data
*
* Writes the contacts formatted according to the VCard specification.
* Delegates the formatting to KABC::VCardConverter
*
* Available through FormatFactory::outputFormat(), name "vcard"
*
* @author Kevin Krammer, <kevin.krammer@gmx.at>
*
* @see @ref formathandling
*/
class VCardOutput : public OutputFormat
{
public:
    VCardOutput();
    virtual ~VCardOutput();

    virtual QString description() const;

    virtual bool setOptions(const QByteArray& options);
    virtual QString optionUsage() const;

    virtual bool setCodec(QTextCodec* codec);

    virtual bool writeAddressee(const KABC::Addressee& addressee, std::ostream& stream);
    virtual bool writeAddresseeList(const KABC::AddresseeList& addresseeList,
                                    std::ostream& stream);

private:
    KABC::VCardConverter* m_converter;
    int m_vCardVersion;
    /**
    * The codec to use for converting QString's UTF encoding to output text
    *
    * @see QTextCodec::fromUnicode()
    */
    QTextCodec* m_codec;
};

/**
* @brief Output formatter for email addresses
*
* Depending on optional parameters, writes each contacts email addresses,
* one per line
*
* Available through FormatFactory::outputFormat(), name "email"
*
* @author Kevin Krammer, <kevin.krammer@gmx.at>
*
* @see @ref formathandling
*/
class EmailOutput : public OutputFormat
{
public:
    EmailOutput();

    virtual QString description() const;

    virtual bool setOptions(const QByteArray& options);
    virtual QString optionUsage() const;

    virtual bool setCodec(QTextCodec* codec);

    virtual bool writeAddressee(const KABC::Addressee& addressee, std::ostream& stream);
    virtual bool writeAddresseeList(const KABC::AddresseeList& addresseeList,
                                    std::ostream& stream);

private:
    /**
    * When @c true write all email addresses of a contact, otherwise just the
    * peferred one
    */
    bool m_allEmails;

    /**
    * When @c true include the contact's name, i.e. "Name <address>", otherwise
    * just the email address (also without angle brackets)
    */
    bool m_includeName;
    /**
    * The codec to use for converting QString's UTF encoding to output text
    *
    * @see QTextCodec::fromUnicode()
    */
    QTextCodec* m_codec;

private:
    /**
    * @brief Helper method for decorating email addresses
    *
    * Handles the adding of the name if #m_includeName is @c true
    *
    * @param addressee the addressee to get the name from when required
    * @param email the email address string to eventually decorate
    *
    * @return the properly formatted email address
    */
    QString decorateEmail(const KABC::Addressee& addressee, const QString& email) const;
};

/**
* @brief Output formatter for use as input of the email client "mutt"
*
* Writes each contact's email addresses formatted for usage in "mutt".
* The option names are also from "mutt", the kabbclient program has
* a special mode so it can be called directly from "mutt"
*
* Available through FormatFactory::outputFormat(), name "mutt"
*
* @author Kevin Krammer, <kevin.krammer@gmx.at>
*
* @see @ref formathandling
*/
class MuttOutput : public OutputFormat
{
public:
    MuttOutput();

    virtual QString description() const;

    virtual bool setOptions(const QByteArray& options);
    virtual QString optionUsage() const;

    virtual bool setCodec(QTextCodec* codec);

    virtual bool writeAddressee(const KABC::Addressee& addressee, std::ostream& stream);
    virtual bool writeAddresseeList(const KABC::AddresseeList& addresseeList,
                                    std::ostream& stream);

private:
    /**
    * When @c true write all email addresses for each contact, one per line,
    * otherwise just the preferred one
    */
    bool m_allEmails;

    /**
    * When @c true use the format "mutt" calls "query", i.e.
    * "emailaddress<tab>name"
    *
    * Example: "kevin.krammer@gmx.at<tab>Kevin Krammer"
    *
    * Otherwise use the format "mutt" calls "alias", i.e.
    * "alias key<tab>Name <emailaddress>" where 'key' depend on option
    * #m_altKeyFormat, #m_preferNickNameKey and #m_alsoNickNameKey
    *
    * Example: "alias kkrammer<tab>Kevin Krammer <kevin.krammer@gmx.at>"
    */
    bool m_queryFormat;

    /**
    * When @c true use what "mutt" calls the "altkey" option of format "alias",
    * i.e. "xyyyy" where 'x' is the first letter of the person's given name and
    * 'yyyy' is the person's family name, lower cased and any space replaced
    * with underscores '_'
    *
    * Example: "kkrammer"
    *
    * Otherwise use "xxxyyy", where 'xxx' are the first three letters of the
    * person's given name and 'yyy' are the first three letters of the person's
    * family name
    *
    * Example: "KevKra"
    * 
    * @see #m_preferNickNameKey, #m_alsoNickNameKey
    */
    bool m_altKeyFormat;


    /**
     * When @c true prefer an existing nickname over the normal alias key.
     *
     * Example: "alias krake Kevin Krammer <kevin.krammer@gmx.at>"
     * instead of "alias KevKra Kevin Krammer <kevin.krammer@gmx.at>" or
     * "alias kkrammer Kevin Krammer <kevin.krammer@gmx.at>" depending on
     * value of #m_altKeyFormat
     * 
     * @see #m_alsoNickNameKey
     */
    bool m_preferNickNameKey;

    /**
     * When @c true add one additional output per address with nickname as the alias key.
     *
     * Example: "alias KevKra Kevin Krammer <kevin.krammer@gmx.at>"
     * and      "alias krake Kevin Krammer <kevin.krammer@gmx.at>"
     * 
     * @see #m_preferNickNameKey
     */
    bool m_alsoNickNameKey;

    /**
    * The codec to use for converting QString's UTF encoding to output text
    *
    * @see QTextCodec::fromUnicode()
    */
    QTextCodec* m_codec;

private:
    /**
    * @brief Helper method for creating the "key" part of the output
    *
    * Create "key" part for the output in format "alias" depending on
    * #m_altKeyFormat
    *
    * @param addressee the contact to get the name from
    *
    * @return the 'key' string
    */
    QString key(const KABC::Addressee& addressee) const;

   /**
    * @brief Helper method for creating the "key" part of nick names
    *
    * Create "key" part for the output in format "alias" using the
    * nick name, if one exists
    *
    * @param addressee the contact to get the name from
    *
    * @return the 'key' string, or 0 if there's no nick name
    */
    QString nickNameKey(const KABC::Addressee& addressee) const;
};

/**
* @brief Output formatter for CSV (comma separated values)
*
* Writes each contact to a line of columns, separated by a delimiter string
* probably quoted if the delimiter is part of a field text.
*
* The actual formatting is delegated to a CSVTemplate instance, fetched
* through CSVTemplateFactory based on a mandatory name parameter.
* See @ref csvhandling
*
* Available through FormatFactory::outputFormat(), name "csv"
*
* @author Kevin Krammer, <kevin.krammer@gmx.at>
*
* @see @ref formathandling
*/
class CSVOutput : public OutputFormat
{
public:
    explicit CSVOutput(CSVTemplateFactory* templateFactory);
    virtual ~CSVOutput();

    virtual QString description() const;

    virtual bool setOptions(const QByteArray& options);
    virtual QString optionUsage() const;

    virtual bool setCodec(QTextCodec* codec);

    virtual bool writeAddressee(const KABC::Addressee& addressee, std::ostream& stream);
    virtual bool writeAddresseeList(const KABC::AddresseeList& addresseeList,
                                    std::ostream& stream);

private:
    /**
    * The codec to use for converting QString's UTF encoding to output text
    *
    * @see QTextCodec::fromUnicode()
    */
    QTextCodec*  m_codec;

    /**
    * The configured template handler for formatting and field mapping.
    * @see @ref cvs-handling
    */
    CSVTemplate* m_template;

    /**
    * The factory for creating the template handler
    */
    CSVTemplateFactory* m_templateFactory;
};

#endif

// End of file

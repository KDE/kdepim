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

#ifndef INPUTFORMATIMPLS_H
#define INPUTFORMATIMPLS_H

// local includes
#include "inputformat.h"

// forward declarations
class CSVTemplate;
class CSVTemplateFactory;
class DialogInputPrivate;

namespace KABC
{
    class VCardConverter;
}

/**
* @brief Input parser for KABC UIDs
*
* Treats input as KABC unique IDs, one per line. See KABC::Addressee::uid()
*
* Available through FormatFactory::inputFormat(), name "uid"
*
* @author Kevin Krammer, <kevin.krammer@gmx.at>
*
* @see @ref formathandling
*/
class UIDInput : public InputFormat
{
public:
    virtual QString description() const;

    virtual bool setOptions(const QByteArray& options);

    virtual bool setCodec(QTextCodec* codec);

    virtual KABC::Addressee readAddressee(std::istream& stream);

private:
    /**
    * The codec to use for converting input text to QString's UTF encoding
    *
    * @see QTextCodec::toUnicode()
    */
    QTextCodec* m_codec;
};

/**
* @brief Input parser for VCard data
*
* Treats input as contact data formatted according to the VCard specification.
* Delegates actual parsing to KABC::VCardConverter
*
* Available through FormatFactory::inputFormat(), name "vcard"
*
* @author Kevin Krammer, <kevin.krammer@gmx.at>
*
* @see @ref formathandling
*/
class VCardInput : public InputFormat
{
public:
    VCardInput();
    virtual ~VCardInput();

    virtual QString description() const;

    virtual bool setOptions(const QByteArray& options);

    virtual bool setCodec(QTextCodec* codec);

    /**
    * @brief Reads a single contact from the input stream
    *
    * Aggregates lines from input until it encounters "END:VCARD".
    * When it does, it delegates parsing of the accumulated string to
    * KABC::VCardConverter::parseVCard()
    *
    * @param stream the standard input stream to read text from
    * @return a KABC::Addressee object containing the read data
    */
    virtual KABC::Addressee readAddressee(std::istream& stream);

private:
    /**
    * VCard parser from libkabc
    */
    KABC::VCardConverter* m_converter;

    /**
    * The codec to use for converting input text to QString's UTF encoding
    *
    * @see QTextCodec::toUnicode()
    */
    QTextCodec* m_codec;
};

/**
* @brief Input parser for email addresses
*
* Treats input as email addresses, one per line.
* Understands pure email addresses, e.g. "kevin.krammer@gmx.at", and addresses
* plus name, e.g. "Kevin Krammer <kevin.krammer@gmx.at>", see
* KABC::Addressee::parseEmailAddress()
*
* Available through FormatFactory::inputFormat(), name "email"
*
* @author Kevin Krammer, <kevin.krammer@gmx.at>
*
* @see @ref formathandling
*/
class EmailInput : public InputFormat
{
public:
    virtual QString description() const;

    virtual bool setOptions(const QByteArray& options);

    virtual bool setCodec(QTextCodec* codec);

    virtual KABC::Addressee readAddressee(std::istream& stream);

private:
    /**
    * The codec to use for converting input text to QString's UTF encoding
    *
    * @see QTextCodec::toUnicode()
    */
    QTextCodec* m_codec;
};

/**
* @brief Input parser for unspecified text queries
*
* Treats a single input line as a search string, meaning it fills in
* the addressee fields email and name, so the text can match either.
*
* @note Since the created addressee is probably filled with unsuitable data
*       (especially the email field), it is not used in Add or Merge operations
*
* Available through FormatFactory::inputFormat(), name "search"
*
* @author Kevin Krammer, <kevin.krammer@gmx.at>
*
* @see @ref formathandling
*/
class SearchInput : public InputFormat
{
public:
    virtual QString description() const;

    virtual bool setOptions(const QByteArray& options);

    virtual bool setCodec(QTextCodec* codec);

    virtual KABC::Addressee readAddressee(std::istream& stream);

private:
    /**
    * The codec to use for converting input text to QString's UTF encoding
    *
    * @see QTextCodec::toUnicode()
    */
    QTextCodec* m_codec;
};

/**
* @brief Input parser for people's names
*
* Treats a single input line as a person's name. See
* KABC::Addressee::setNameFromString()
*
* Available through FormatFactory::inputFormat(), name "name"
*
* @author Kevin Krammer, <kevin.krammer@gmx.at>
*
* @see @ref formathandling
*/
class NameInput : public InputFormat
{
public:
    virtual QString description() const;

    virtual bool setOptions(const QByteArray& options);

    virtual bool setCodec(QTextCodec* codec);

    virtual KABC::Addressee readAddressee(std::istream& stream);

private:
    /**
    * The codec to use for converting input text to QString's UTF encoding
    *
    * @see QTextCodec::toUnicode()
    */
    QTextCodec* m_codec;
};

/**
* @brief Input parser for CSV (comma separated values)
*
* Treats each line as a record composed columns, where a column is separated
* from the next column by a separator string, often a comma.
*
* The actual parsing (splitting, unquoting, etc) is delegated to a
* CSVTemplate instance, fetched through CSVTemplateFactory based on a
* mandatory name parameter. See @ref csvhandling
*
* Available through FormatFactory::inputFormat(), name "csv"
*
* @author Kevin Krammer, <kevin.krammer@gmx.at>
*
* @see @ref formathandling
*/
class CSVInput : public InputFormat
{
public:
    explicit CSVInput(CSVTemplateFactory* templateFactory);

    virtual QString description() const;

    virtual bool setOptions(const QByteArray& options);
    virtual QString optionUsage() const;

    virtual bool setCodec(QTextCodec* codec);

    virtual KABC::Addressee readAddressee(std::istream& stream);

private:
    /**
    * The codec to use for converting input text to QString's UTF encoding
    *
    * @see QTextCodec::toUnicode()
    */
    QTextCodec*  m_codec;

    /**
    * The configured template handler for parsing and field mapping.
    * @see @ref cvs-handling
    */
    CSVTemplate* m_template;

    /**
    * The factory for creating the template handler
    */
    CSVTemplateFactory* m_templateFactory;

private:
    /**
    * @brief Helper method for splitting and unescaping
    *
    * Gets its format information, i.e. quoting and delimiter, from
    * the template handler #m_template
    *
    * @param values a line from the input
    *
    * @return a list of column values
    */
    QStringList split(const QString& values) const;
};

/**
* @brief Input selection through a GUI dialog
*
* Does not use any text input at all but opens the standard KDE addressee
* selection dialog.
* Shows dialog on the first call for selection and returns the first selected
* contact. Just returns any further selected contacts on subsequent calls.
*
* @note Since it does not process any input at all it is not used in Add or
*       Merge operations
*
* Available through FormatFactory::inputFormat(), name "dialog"
*
* @author Kevin Krammer, <kevin.krammer@gmx.at>
*
* @see @ref formathandling
*/
class DialogInput : public InputFormat
{
public:
    DialogInput();
    virtual ~DialogInput();

    virtual QString description() const;

    virtual bool setOptions(const QByteArray& options);

    virtual bool setCodec(QTextCodec* codec);

    virtual KABC::Addressee readAddressee(std::istream& stream);

private:
    /**
    * "D" pointer
    */
    DialogInputPrivate* m_private;
};

#endif

// End of file

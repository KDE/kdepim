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

#ifndef CSVTEMPLATE_H
#define CSVTEMPLATE_H

// Qt includes
#include <QtCore/QMap>
#include <QtCore/QString>

// forward declarations
class KConfigBase;
class QDateTime;

namespace KABC
{
    class Addressee;
}

/**
* @brief Class for handling KAddressBook's CSV templates
*
* Reads the template's column mapping and configuration like quoting and
* delimiter from a KConfigBase object that has parsed the template's
* @c .desktop file.
*
* The instance can then be used to get or set the value of a addressee field
* based on the column index of the target CSV format.
*
* Example: Creating a CSV line from an addressee object based on a template.
* Using the default template instead of creating one. See CSVTemplateFactory
* for an easy way to do that.
* @code
* QString line;
*
* CSVTemplate* template = CSVTemplate::defaultTemplate();
*
* for (int col = 0; i < template->columns(); ++i)
* {
*     line += template->quote(); // start quoting of the field
*
*     line += template->fieldText(col, addressee); // add the text value
*
*     line += template->quote(); // end quoting of the field
*
*     // add delimiter if not at end of the CSV line
*     if (col < (template->columns() - 1))
*         line += template->delimiter();
* }
* @endcode
*
* Example: Filling an addressee instance from a line of a CSV file.
* Assuming for simplicity that there is no quoting.
* @code
* KABC::Addressee addressee;
*
* // as we assumed absence of quoting, we can use QStringList::split()
* // to separate the fields. Using @c true for allowEmptyEntries to make sure
* // we do not miss any empty fields
* QStringList fields = QStringList::split(template->delimiter(), line, true);
*
* // iterate over the fields and let the template parse the text
* QStringList::const_iterator it = fields.begin();
* for (int col = 0; it != fields.end(); ++it, ++col)
* {
*     template->setFieldText(col, addressee, *it);
* }
* @endcode
*
* @author Kevin Krammer, <kevin.krammer@gmx.at>
* @see CSVTemplateFactory
*/
class CSVTemplate
{
public:
    /**
    * @brief Creates a template handler for the given description
    *
    * Assuming that the best way to read the actual template description files
    * is using KConfigBase derived classes, this constructor initializes a
    * template handler from such an instance.
    *
    * @see csvhandling
    */
    explicit CSVTemplate(KConfigBase* config);

    /**
    * @brief Returns the number of CSV columns
    *
    * The returned value is taken from the @c "Columns" entry of group
    * @c "General" of the CSV template.
    *
    * It can be greater than the number of specified column<->field mappings
    * for example for templates that only parse parts of a larger CSV format.
    *
    * @return the number of columns of the CSV format
    */
    inline int columns() const { return m_columns; }

    /**
    * @brief Returns the CSV delimiter string
    *
    * The delimiter is usually only one character, e.g comma, semicolon, etc.
    * However the delimiter can also be the value of an entry in the CSV
    * template and thus theoretially be longer.
    *
    * @note The delimiter could appear inside CSV columns if the column text is
    * inside quotes. See quote()
    *
    * The actual value is determined based on the @c "DelimiterType" entry of
    * group @c "General" of the CSV template:
    * - value @c 0: comma @c ','
    * - value @c 1: semicolon @c ';'
    * - value @c 2: tab character @c '\\t'
    * - value @c 3: space @c ' '
    * - value @c 4: value of the @c "DelimiterOther" entry of the same group
    *
    * @return the delimiter string of the CSV format
    */
    inline const QString& delimiter() const { return m_delimiter; }

    /**
    * @brief Returns the CSV quoting string
    *
    * Quoting can be necessary if a CSV column text can potentially contain
    * the delimiter character/string.
    * For example
    * @code
    * one,"two,2",three
    * @endcode
    * would be three columns if the CSV format's quoting character is the
    * double quote character but four columns if it is single quote character
    * or if the template doesn't specify a quoting character.
    *
    * The actual value is determined based on the @c "QuoteType" entry of
    * group @c "General" of the CSV template:
    * - value @c 0: double quote @c "
    * - value @c 1: single quote @c '
    * - value @c 2: no quoting
    *
    * @return the quoting character/string. Can be @c QString()
    *
    * @see delimiter()
    */
    inline const QString& quote() const { return m_quote; }

    /**
    * @brief Returns the specified field of the given addressee formatted
    *        as a string
    *
    * Uses the column to field index mapping specified in the group
    * <tt>"csv column map"</tt> to determine the @p addressee field.
    * Converts the field's datatype to a string representation if necessary,
    * e.g. a date string according to the CSV format's date format spec.
    *
    * In case of a date the value of the @c "DatePattern" entry of group
    * @c "General" is used as a hint how the date is supposed to look
    * in the CSV data.
    *
    * @note If the text contains the delimiter() string it will need to be quoted.
    *
    * @param column the CSV column the text is for
    * @param addressee the addressbook entry to take the data from
    *
    * @return text representation of the specified addressee field's value.
    *         Can be @c QString() if the @p column is not mapped or
    *         the @p addressee or field is empty
    *
    * @see setFieldText()
    * @see quote()
    * @see csvhandling
    */
    QString fieldText(int column, const KABC::Addressee& addressee) const;

    /**
    * @brief Sets an addressee field using the data of a given text
    *
    * Uses the column to field index mapping specified in the group
    * <tt>"csv column map"</tt> to determine the @p addressee field.
    * Converts the string representation to the field's datatype to if
    * necessary, e.g. parsing a date string according to the CSV format's date
    * format spec.
    *
    * In case of a date the value of the @c "DatePattern" entry of group
    * @c "General" is used as a hint how the date is supposed to look
    * in the CSV data, i.e. which part of the string should be parsed as which
    * date value.
    *
    * @note The string is expected without any quoting characters. See quote()
    *
    * @param column the CSV column the text is from. If there is no mapping for
    *        this column, it will just return
    * @param addressee the addressbook entry to put the data into
    * @param text the data from the CSV line
    *
    * @see fieldText()
    * @see csvhandling
    */
    void setFieldText(int column, KABC::Addressee& addressee, const QString& text) const;

    /**
    * @brief Returns the template with default setup
    *
    * The default template contains a mapping for almost all fields of the
    * KABC::Addressee, it just leaves formatted name out, i.e. column @c 0
    * will be the family name, column @c 1 will be the given name, and so on.
    *
    * The quote character will be double quote and the delimiter the comma.
    *
    * Dateformat spec is @c "Y-M-D", thus resembling the format specified for
    * @c ISO-8601 (e.g. 2005-10-31)
    *
    * @note There will only be one instance of this template, i.e.
    *       defaultTemplate() is like a singleton
    *
    * @return the default CSV template
    */
    static CSVTemplate* defaultTemplate();

private:
    /**
    * Number of columns of the associated CSV format.
    * @see columns()
    */
    int m_columns;

    /**
    * The string that separates columns in the associated CSV format.
    * @see delimiter()
    */

    QString m_delimiter;

    /**
    * The string to use for quoting columns that contain the delimiter string.
    * @see quote()
    */
    QString m_quote;

    /**
    * A mapping from column index to field of KABC::Addressee
    */
    QMap<int, int> m_columnToField;

    /**
    * A pattern to indicate how dates look like in their text form.
    * @see createDateFormat()
    */
    QString m_datePattern;

    /**
    * A date format string usable with QDateTime.
    * @see createDateFormat
    */
    QString m_dateFormat;

    /**
    * A pointer to the singleton instance of the default template or @c 0
    * if default template has not been requested yet.
    * @see defaultTemplate()
    */
    static CSVTemplate* m_defaultTemplate;

private:
    /**
    * @brief Creates a CSV template with a given date pattern
    *
    * This is used internally to create the default template.
    * Calls createDateFormat()
    */
    explicit CSVTemplate(const QString& datePattern);

    /**
    * @brief Formats a given date according to the template's date pattern
    *
    * Actually uses the date format created by createDateFormat()
    *
    * @param date the date to format
    * @return the formatted date or @c QString() if @p date is not valid
    *
    * @see QDateTime::toString()
    * @see QDateTime::isValid()
    */
    QString formatDate(const QDateTime& date) const;

    /**
    * @brief Parses a given string for a date according to the template's date
    *        pattern
    *
    * Any character in the pattern that has no special meaning, i.e. anything
    * other than Y,y,M,m,D,d, has to match exactly, for example a pattern
    * @c "Y--M" would correctly parse @c "2005--11" but not @c "2005-11"
    *
    * @param text the text to parse
    * @return the parsed date or a null date if parsing fails
    *
    * @see QDateTime::isNull()
    */
    QDateTime parseDate(const QString& text) const;

    /**
    * @brief Creates a QDateTime::toString suitable format string
    *
    * Takes the CSV template's date pattern and creates an equivalent date
    * format string to be suitable for QDateTime::toString().
    *
    * It basically expands the date pattern's special character to the correct
    * number of equivalent date format characters.
    */
    void createDateFormat();
};

/**
* @defgroup csvhandling Handling of CSV (comma separated values) input/output
*
* A CSV template is basically an INI-style text file with the .desktop file
* extension.
*
* The file consists of three sections:
*
* - @c [General]: describes formatting options, like how dates should be
*               interpreted or formatted, or what character is used as the
*               field delimiter
*
* - @c [Misc]: name of the template
*
* - <tt>[csv column map]:</tt> describes which field or column of the CSV data
*                            has which addressbook field
*
* Example:
* @code
* [General]
* Columns=3
* DatePattern=Y-M-D
* DelimiterType=0
* QuoteType=0
*
* [Misc]
* Name=Example CSV template
*
* [csv column map]
* 0=1
* 1=2
* 2=7
* @endcode
* This would transform the following input
* <tt>"Doe","John","2005-07-25"</tt>
* into an addressbook entry for a person named John Doe, having been born on
* July 25th, 2005
*
* Details:
* @code
* Columns=3
* @endcode
* Says that the CSV data will have three fields
*
* @code
* DatePattern=Y-M-D
* @endcode
* Says that dates are to be interpreted as four digit year followed by two
* digit month followed by a two digit day, separated by @c '-'
*
* Other possible letters are:
* - @c 'y' (two digit year using 19 as the century)
* - @c 'm' (one digit month for months < 10)
* - @c 'd' (one digit day for days < 10)
*
* @code
* DelimiterType=0
* @endcode
* Use comma as the field separator.
*
* Other possible values are:
* - @c 1 (semicolon)
* - @c 2 (tab)
* - @c 3 (single space)
* - @c 4 (user defined)
*
* In case of @c DelimiterType=4 there is an additional entry @c DelimiterOther
* which has the separator character as its value.
*
* @c DelimiterType=4 followed by @c DelimiterOther=, would for example be
* equivalent to @c DelimiterType=0
*
* @code
* QuoteType=0
* @endcode
* Use standard double quote character @c " as the field quoting.
* Other possible values are:
* - @c 1 (use single quote @c ' )
* - @c 2 no quoting
*
* For full examples see the files installed along KAddressBook.
*
* The column map tells the filters which columns of the CSV data maps to which
* field in the address book. Column index starts at @c 0
*
* The above example template says that the first column is the family name, the
* second column is the given name or first name and that the thrid and last
* columns is the birthday date.
*
* @code
* 00 "Formatted Name"
* 01 "Family Name"
* 02 "Given Name"
* 03 "Additional Names"
* 04 "Honorific Prefixes"
* 05 "Honorific Suffixes"
* 06 "Nick Name"
* 07 "Birthday"
* 08 "Home Address Street"
* 09 "Home Address Locality"
* 10 "Home Address Region"
* 11 "Home Address Postal Code"
* 12 "Home Address Country"
* 13 "Home Address Label"
* 14 "Business Address Street"
* 15 "Business Address Locality"
* 16 "Business Address Region"
* 17 "Business Address Postal Code"
* 18 "Business Address Country"
* 19 "Business Address Label"
* 20 "Home Phone"
* 21 "Business Phone"
* 22 "Mobile Phone"
* 23 "Home Fax"
* 24 "Business Fax"
* 25 "Car Phone"
* 26 "Isdn"
* 27 "Pager"
* 28 "Email Address"
* 29 "Mail Client"
* 30 "Title"
* 31 "Role"
* 32 "Organization"
* 33 "Note"
* 34 "URL"
*
* Values below currently not supported by CSVTemplate::setFieldText() or
* CSVTemplate::fieldText()
*
* 35 "Department"
* 36 "Profession"
* 37 "Assistant's Name"
* 38 "Manager's Name"
* 39 "Spouse's Name"
* 40 "Office"
* 41 "IM Address"
* 42 "Anniversary"
* @endcode
*/

#endif

// End of file

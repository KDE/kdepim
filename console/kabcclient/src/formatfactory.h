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

#ifndef FORMATFACTORY_H
#define FORMATFACTORY_H

// Qt includes
#include <qcstring.h>
#include <qvaluelist.h>

// forward declarations
class CSVTemplateFactory;
class InputFormat;
class OutputFormat;

/**
* A list of strings for format names
*/
typedef QValueList<QCString> QCStringList;

/**
* @defgroup formathandling Handling of various input and output formats
*
* In order to be useful address data input and output has to be possible
* in various formats, for example full VCards or just email addresses.
*
* The three main classes involved in the format handling are:
* - FormatFactory: creating format implementation instances
* - InputFormat: base interface for input format handlers
* - OutputFormat: base interface for output format handlers
*
* Implementations of the InputFormat interface read the text data from
* an C++ input stream and parse it for contact data according to the
* format they implement.
*
* Implementations of the OutputFormat interface format the contact data
* into a pure text form according to the format they implement and then
* write this to a C++ output stream
*
* Both kinds of implementations need to be added to the FormatFactory
* so they can be created by name.
*
* @example converter.cpp
*/

/**
* @brief Factory for input parsers and output formatters
*
* The factory can be queried for the InputFormat and OutputFormat
* implementations it knows and can create.
*
* Example: displaying all format names and their respective description
* @code
* FormatFactory factory;
*
* QCStringList inputFormats = factory.inputFormatList();
* QCStringList::const_iterator it = inputFormats.being();
* for (; it != inputFormats.end(); ++it)
* {
*     InputFormat* inputFormat = factory.inputFormat(*it);
*     cout << *it << endl;
*     cout << inputFormat->description().local8Bit() << endl;
*     delete inputFormat;
* }
*
* QCStringList outputFormats = factory.outputFormatList();
* it = outputFormats.being();
* for (; it != outputFormats.end(); ++it)
* {
*     OutputFormat* outputFormat = factory.outputFormat(*it);
*     cout << *it << endl;
*     cout << outputFormat->description().local8Bit() << endl;
*     delete outputFormat;
* }
* @endcode
*
* @author Kevin Krammer, <kevin.krammer@gmx.at>
*
* @see InputFormat
* @see OutputFormat
*/
class FormatFactory
{
public:
    /**
    * @brief Creates and initializes the factory
    */
    FormatFactory();

    /**
    * @brief Destroys the factory and its internal data
    */
    ~FormatFactory();

    /**
    * @brief Returns a list of input parser names
    *
    * Each list entry is a simple string that can be used to identify
    * the parser on the commandline.
    * The factory method inputFormat() will check for exactly those strings.
    *
    * @return a list of input parser names
    *
    * @see inputFormat()
    * @see InputFormat
    * @see outputFormatList()
    */
    inline QCStringList inputFormatList() const  { return m_inputFormats;  }

    /**
    * @brief Returns a list of output formatter names
    *
    * Each list entry is a simple string that can be used to identify
    * the formatter on the commandline.
    * The factory method outputFormat() will check for exactly those strings.
    *
    * @return a list of output formatter names
    *
    * @see outputFormat()
    * @see OutputFormat
    * @see inputFormatList()
    */
    inline QCStringList outputFormatList() const { return m_outputFormats; }

    /**
    * @brief Creates an InputFormat instance for the given name
    *
    * @warning Every call creates a new instance and the caller gains its
    * ownership, i.e. has to delete it when it isn't used any longer.
    *
    * @param name the input format name, as taken from the commandline
    * @return a new InputFormat instance of the input parser associated with
    *         the given name or @c 0 if the name is unkown to the factory
    *
    * @see inputFormatList()
    * @see InputFormat
    * @see outputFormat
    */
    InputFormat* inputFormat(const QCString& name);

    /**
    * @brief Creates an OutputFormat instance for the given name
    *
    * @warning Every call creates a new instance and the caller gains its
    * ownership, i.e. has to delete it when it isn't used any longer.
    *
    * @param name the output format name, as taken from the commandline
    * @return a new OutputFormat instance of the output formatter associated
    *         with the given name or @c 0 if the name is unkown to the factory
    *
    * @see inputFormatList()
    * @see InputFormat
    * @see outputFormat
    */
    OutputFormat* outputFormat(const QCString& name);

private:
    /**
    * The list of the known InputFormat names
    */
    QCStringList m_inputFormats;

    /**
    * The list if the known OutputFormat names
    */
    QCStringList m_outputFormats;

    /**
    * Factory for the templates of the CSV Input and Output format
    * implementations
    *
    * @see CSVTemplate
    */
    CSVTemplateFactory* m_csvtemplateFactory;

private:
    FormatFactory(const FormatFactory&);
    FormatFactory& operator=(const FormatFactory&);
};

#endif

// End of file

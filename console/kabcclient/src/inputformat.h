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
//  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifndef INPUTFORMAT_H
#define INPUTFORMAT_H

// standard includes
#include <istream>

// forward declarations
class QTextCodec;

namespace KABC
{
    class Addressee;
};

/**
* @short Interface for input format parsers
*
* This is the interface for input format parsers.
*
* @note Implementations should return a KABC::Addressee object on each call to
* their readAddressee() method. If the parsing failes they are supposed to
* return an empty object.
* readAddressee() will be called repeatetly until the input stream is at
* its end (istream::eof() returns @c true) or it the stream is in condition bad
* (istream::bad() returns @c true)
*
* @author Kevin Krammer, <kevin.krammer@gmx.at>
* @see OutputFormat
*/
class InputFormat
{
public:
    /**
    * @short Destroys the instance
    * Defined here because the class contains virtual methods
    */
    virtual ~InputFormat() {}

    /**
    * @short Returns a translate description of the input format
    *
    * Returns a translated description of the parser and its
    * general capabilities.
    *
    * @return a short descriptive string what kind of input format it can handle
    *
    * @note Implementations should not include the optional settings a
    * parser understands, this is what optionUsage() is for.
    * As the description is displayed after the format name, it is recommended
    * to add two tabs after each newline for aligned output
    */
    virtual QString description() const = 0;

    /**
    * @short Reads one addressee from the input stream
    *
    * Creates a single KABC::Addressee object from the data available in
    * the given input stream according to the parsers formatting rules.
    *
    * @param stream the standard input stream to read text from
    * @return a KABC::Addressee object containing the read data
    *
    * @see KABC::Addressee
    * @see setCodec
    * @see std::ios_base
    *
    * @note This method is called until the stream is either at its end or
    * gone bad, so if an implementation has more than one addressee to return
    * it has to make sure neither of this conditions is met.
    * If the data available in the stream is not sufficient to create an
    * addressee according to the implementations format rules, it should
    * return an empty object, i.e. the one creates by the Addressee class'
    * default constructor.
    *
    */
    virtual KABC::Addressee readAddressee(std::istream& stream) = 0;

    /**
    * @short Configures the input format
    *
    * Sets parser options, i.e. format specific settings that change how the
    * input format treats the input text.
    *
    * @param options a string as taken from the commandline
    * @return @c false if the options are not valid or if the input format
    *         doesn't support options. @c true if the options where valid
    *
    * @note Implementations can use any format in their options string.
    * However it is recommended to stay consistent with the other format
    * implementations and use a comma separated list
    */
    virtual bool setOptions(const QCString& options) = 0;

    /**
    * @short Returns a translate message about the available format options
    *
    * The option description string contains each option and its respective
    * description for displaying to the user.
    *
    * @return a short description of each option or @c QString::null if
    *         the input format does not allow configuration options
    *
    * @note Implementations can return any formatting in the string, but
    * it is recommended to stay consistent with the other format
    * implementations and return the following format:
    * one line per option and each line formatted like this
    * @code
    * option-name tabs option-description
    * @endcode
    * where tabs is either one or two tab characters depending on the length
    * of the option-name, e.g. two tabs for length < 8
    * Default implementation returns @c QString::null
    */
    virtual QString optionUsage() const { return QString::null; }

    /**
    * @short Sets the text codec to use
    *
    * This allows to have the text read from the input stream
    * interpreted according to a specific text encoding.
    * Depending on the format's specifications not all technically available
    * codecs might be valid
    *
    * @warning Always set a codec, the input formats need it!
    *
    * @param codec the text encoding handler to use
    * @return @c true if the input format accepts this codec. i.e. can work
    *         with it reasonably. Otherwise returns @c false
    */
    virtual bool setCodec(QTextCodec* codec) = 0;
};

#endif

// End of file

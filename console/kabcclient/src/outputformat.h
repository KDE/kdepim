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

#ifndef OUTPUTFORMAT_H
#define OUTPUTFORMAT_H

// standard includes
#include <ostream>

// forward declarations
namespace KABC
{
    class Addressee;
    class AddresseeList;
};

/**
* @short Interface for output formatters
*
* This is the interface for output formatters.
* Output formats can used to write only parts of the an addressee object's
* data to the output stream or to format the data in a specific way.
*
* @note Implementations can call writeAddressee() from writeAddresseeList() if
* that works for their format restrictions.
* If writeAddressee() is called from an outside caller, it can savely assume
* that there is no relation between two calls, i.e. the two calls belong to
* different operations. So in case an implementation needs to write some
* header or footer it can do so in both writeAddressee() and
* writeAddresseeList().
*
* @author Kevin Krammer, <kevin.krammer@gmx.at>
* @see InputFormat
*/
class OutputFormat
{
public:
    /**
    * @short Destroys the instance
    * Defined here because the class contains virtual methods
    */
    virtual ~OutputFormat() {}


    /**
    * @short Returns a translate description of the output format
    *
    * Returns a translated description of the formatter and its
    * general capabilities.
    *
    * @return a short descriptive string how addressee data will be formatted
    *
    * @note Implementations should not include the optional settings a
    * formatter understands, this is what optionUsage() is for.
    * As the description is displayed after the format name, it is recommended
    * to add two tabs after each newline for aligned output
    */
    virtual QString description() const = 0;

    /**
    * @short Configures the output format
    *
    * Sets formatter options, e.g. which parts of the addressee data to use
    * or which markup to apply.
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
    *         the output format does not allow configuration options
    *
    * @note Implementations can return any formatting in the string, but
    * it is recommended to stay consistent with the other format
    * implementations and return the following format:
    * one line per option and each line formatted like this
    * @code
    * option-name tabs option-description
    * @endcode
    * where tabs is either one or two tab characters depending on the length
    * of the option-name, e.g. two tabs for length < 8.
    * Default implementation returns @c QString::null
    */
    virtual QString optionUsage() const { return QString::null; }

    /**
    * @short Sets the text codec to use
    *
    * This allows to have the text recoded to a specific text encoding.
    * Depending on the format's specifications not all technically available
    * codecs might be valid. e.g. VCards in version 3.0 are expected to be
    * encoded in UTF-8
    *
    * @warning Always set a codec, the output formats need it!
    *
    * @param codec the text encoding handler to use
    * @return @c true if the output format accepts this codec. i.e. can work
    *         with it reasonably. Otherwise returns @c false
    */
    virtual bool setCodec(QTextCodec* codec) = 0;

    /**
    * @short Writes the data of a given addressee to the given output stream
    *
    * Depending on the formatter and its settings it will write all or portions
    * of the available addressee data to the @p stream.
    *
    * @warning a format could require either header or footer around related
    * addressees so use this @em only for single addressee output, i.e. not
    * when iterating over a list of related addressees.
    *
    * @param addressee the addressee object to take data from
    * @param stream the standard output stream to write to
    *
    * @note Implementations that do not need to handle addressee relations
    * can of course implement the writing in this method and just call
    * it from writeAddresseeList() when iterating over the list
    *
    * @see writeAddresseeList()
    * @see KABC::Addressee
    * @see setCodec()
    * @see std::ostream
    */
    virtual bool writeAddressee(const KABC::Addressee& addressee, std::ostream& stream) = 0;

    /**
    * @short Writes the data from each addressee in the given list to the given
    *        output stream
    *
    * Depending on the formatter and its settings it will write all or portions
    * of the available addressee data to the @p stream.
    *
    * @param addresseeList a list of addressee objects
    * @param stream the standard output stream to write to
    *
    * @note Implementations that do not need to handle each addressee
    * differently for example numbering them, can call the writeAddressee()
    * method on each entry of the given list
    *
    * @see writeAddressee()
    * @see KABC::Addressee
    * @see setCodec()
    * @see std::ostream
    */
    virtual bool writeAddresseeList(const KABC::AddresseeList& addresseeList,
                                    std::ostream& stream) = 0;
};

#endif

// End of file

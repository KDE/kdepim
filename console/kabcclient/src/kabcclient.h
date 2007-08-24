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

#ifndef KABCCLIENT_H
#define KABCCLIENT_H

// standard includes
#include <istream>

// Qt includes
#include <QtCore/QObject>

// forward declarations
class FormatFactory;
class InputFormat;
class OutputFormat;
class QTextCodec;

namespace KABC
{
    class AddressBook;
    class Addressee;
    class Picture;
}

/**
* @brief Main handler of the program
*
* This class is the "program", it gets configured with the options passed
* by the user at the command line, retrieves the necessary components from its
* factories (see FormatFactory) and then executes the desired #Operation
*
* @author Kevin Krammer, <kevin.krammer@gmx.at>
*/
class KABCClient: public QObject
{
    Q_OBJECT

public:
    /**
    * @brief List of supported operations
    */
    enum Operation
    {
        /**
        * @brief Writes all contacts of the address book
        *
        * Does not consume any input
        */
        List = 0,

        /**
        * @brief Adds the input to the address book
        *
        * Reads contacts from the input stream in a loop and tries to add each
        * one to the address book.
        *
        * Writes the each contacts data to the output stream.
        *
        * @see KABC::AddressBook::insertAddressee()
        * @see @ref formathandling
        */
        Add,

        /**
        * @brief Removes matching contact from the address book
        *
        * Reads contacts from the input stream in a loop and checks for each
        * which entries in the address book match. If there is more than one
        * match, it will not remove any of them. Else (only one match) it will
        * remove the match from the address book.
        *
        * Can use SearchInput and DialogInput
        *
        * Writes the remove contact's data to the output stream
        *
        * @see KABC::AddressBook::removeAddressee()
        * @see @ref formathandling
        */
        Remove,

        /**
        * @brief Merges input data into the address book
        *
        * Reads contacts from the input stream in a loop and checks for each
        * which entries in the address book match. If there is more than one
        * match, it will not attempt to merge. Else (only one match) it will
        * use the found contact and merge information from the input contact
        * into it and then replace the one inside the address book with the
        * merged one.
        *
        * Writes the merged contact's data to the output stream.
        *
        * @see KABC::AddressBook::insertAddressee()
        * @see @ref formathandling
        */
        Merge,

        /**
        * @brief Searches for matching entries in the address book
        *
        * Reads contacts from the input stream in a loop and checks for each
        * which entries in the address book match.
        *
        * Can use SearchInput and DialogInput
        *
        * Writes all matches per input to the output stream.
        */
        Search
    };

    /**
    * @brief Creates and initializes the instance
    *
    * @param operation the operation to perform on the address book
    * @param factory the factory to get the input and output format handlers from
    */
    KABCClient(Operation operation, FormatFactory* factory);

    /**
    * @brief Destroys the instance
    */
    virtual ~KABCClient();

    /**
    * @brief Sets the input format to use
    *
    * Checks if the given @p name is a valid input format for the #Operation
    * specified at the construction.
    * If it is not blacklisted, the respective InputFormat will be retrieved
    * from the FormatFactory.
    *
    * @param name the name of an InputFormat to use for parsing the input data
    *
    * @return @c true if the format is allowed for the selected #Operation and
    *         its format parser can be created, otherwise @c false
    *
    * @see setInputOptions()
    * @see setInputCodec()
    * @see setOutputFormat()
    */
    bool setInputFormat(const QByteArray& name);

    /**
    * @brief Sets the output format to use
    *
    * Retrieves the respective OutputFormat from the FormatFactory
    *
    * @param name the name of an OutputFormat to use for formatting the output
    *             data
    *
    * @return @c true if the format has been created, otherwise @c false
    *
    * @see setOutputOptions()
    * @see setOutputCodec()
    * @see setInputFormat()
    */
    bool setOutputFormat(const QByteArray& name);

    /**
    * @brief Sets the options for the input format
    *
    * Passes the @p options to the InputFormat set with setInputFormat()
    *
    * @param options the options for the InputFormat
    *
    * @return @c true if the InputFormat accepts the options, @c false if it
    *         doesn't or if there is not InputFormat set
    *
    * @see InputFormat::setOptions()
    * @see setInputCodec()
    */
    bool setInputOptions(const QByteArray& options);

    /**
    * @brief Sets the options for the output format
    *
    * Passes the @p options to the OutputFormat set with setOutputFormat()
    *
    * @param options the options for the OutputFormat
    *
    * @return @c true if the OutputFormat accepts the options, @c false if it
    *         doesn't or if there is not OutputFormat set
    *
    * @see OutputFormat::setOptions()
    * @see setOutputCodec()
    */
    bool setOutputOptions(const QByteArray& options);

    /**
    * @brief Sets the text codec for reading the input data
    *
    * Translates @c utf, @c utf8, @c utf-8 to @c UTF-8 and @c local, @c locale
    * the codec for the current locale.
    *
    * @param name the name of the QTextCodec. See QTextCodec::codecForName()
    *
    * @return return value of QTextCodec::codecForName()
    *
    * @see setInputFormat()
    */
    bool setInputCodec(const QByteArray& name);

    /**
    * @brief Sets the text codec for writing the output data
    *
    * Translates @c utf, @c utf8, @c utf-8 to @c UTF-8 and @c local, @c locale
    * the codec for the current locale.
    *
    * @param name the name of the QTextCodec. See QTextCodec::codecForName()
    *
    * @return return value of QTextCodec::codecForName()
    *
    * @see setOutputFormat()
    */
    bool setOutputCodec(const QByteArray& name);

    /**
    * @brief Sets the input stream to read data from
    *
    * Depending on the input mode this can be either a @c stringstream on
    * the additional command arguments or @c cin
    *
    * @param stream the input stream for reading
    */
    void setInputStream(std::istream* stream);

    /**
    * @brief Checks if #Operation setup is correct and schedules execution
    *
    * Loads the KABC::StandardAddressBook syncronously and deactivates its
    * "auto save", so the user can operate the program in "simulate" mode
    *
    * @return @c true when #Operation can start, otherwise @c false
    */
    bool initOperation();

    /**
    * @brief Sets the string matching mode
    *
    * Default if @c Qt::CaseInsensitive
    *
    * @param sensitivity whether to do string comparisons case sensitive or not
    */
    inline void setMatchCaseSensitivity(Qt::CaseSensitivity sensitivity)
    {
        m_matchCaseSensitivity = sensitivity;
    }

    /**
    * @brief Sets the save behavior
    *
    * When saving is @p on to address book data will be written to the
    * address book storage, otherwise they will just be performed on the
    * in-memory data.
    *
    * Default is @c true
    *
    * @param on when @c true write data to address book store, when @c false
    *        operate in "simulate" mode
    */
    inline void setAllowSaving(bool on) { m_allowSaving = on; }

private:
    Operation m_operation;

    FormatFactory* m_formatFactory;

    InputFormat*  m_inputFormat;
    OutputFormat* m_outputFormat;

    QTextCodec* m_inputCodec;
    QTextCodec* m_outputCodec;

    KABC::AddressBook* m_addressBook;

    std::istream* m_inputStream;

    Qt::CaseSensitivity m_matchCaseSensitivity;
    bool m_allowSaving;

private:
    int performAdd();
    int performRemove();
    int performMerge();
    int performList();
    int performSearch();

    void mergeAddressees(KABC::Addressee& master, const KABC::Addressee& slave);
    void mergePictures(KABC::Picture& master, const KABC::Picture slave);

    QTextCodec* codecForName(const QByteArray& name);

private slots:
    void slotAddressBookLoaded();
};

#endif

// End of file

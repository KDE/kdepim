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

#ifndef KABCCLIENT_H
#define KABCCLIENT_H

// standard includes
#include <istream>

// Qt includes
#include <qobject.h>

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
};

class KABCClient: public QObject
{
    Q_OBJECT

public:
    enum Operation
    {
        List = 0,
        Add,
        Remove,
        Merge,
        Search
    };
    
    KABCClient(Operation operation, FormatFactory* factory);

    virtual ~KABCClient();

    bool setInputFormat(const QCString& name);
    bool setOutputFormat(const QCString& name);

    bool setInputOptions(const QCString& options);
    bool setOutputOptions(const QCString& options);

    bool setInputCodec(const QCString& name);
    bool setOutputCodec(const QCString& name);
    
    void setInputStream(std::istream* stream);
    
    bool initOperation();

    inline void setMatchCaseSensitive(bool on) { m_matchCaseSensitive = on; }

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

    bool m_matchCaseSensitive;
    bool m_allowSaving;
        
private:
    int performAdd();
    int performRemove();
    int performMerge();
    int performList();
    int performSearch();

    void mergeAddressees(KABC::Addressee& master, const KABC::Addressee& slave);
    void mergePictures(KABC::Picture& master, const KABC::Picture slave);

    QTextCodec* codecForName(const QCString& name);
    
private slots:
    void slotAddressBookLoaded();
};

#endif

// End of file

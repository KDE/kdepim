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

class OutputFormat
{
public:
    virtual ~OutputFormat() {}

    virtual QString description() const { return QString::null; }
    
    virtual bool setOptions(const QCString& options) = 0;
    virtual QString optionUsage() const { return QString::null; }

    virtual bool setCodec(QTextCodec* codec) = 0;
    
    virtual bool writeAddressee(const KABC::Addressee& addressee, std::ostream& stream) = 0;
    virtual bool writeAddresseeList(const KABC::AddresseeList& addresseeList,
                                    std::ostream& stream) = 0;
};

#endif

// End of file

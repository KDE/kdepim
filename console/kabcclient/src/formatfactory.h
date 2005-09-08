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

#ifndef FORMATFACTORY_H
#define FORMATFACTORY_H

// Qt includes
#include <qcstring.h>
#include <qvaluelist.h>

// forward declarations
class CSVTemplateFactory;
class InputFormat;
class OutputFormat;

typedef QValueList<QCString> QCStringList;

class FormatFactory
{
public:
    FormatFactory();
    ~FormatFactory();

    inline QCStringList inputFormatList() const  { return m_inputFormats;  }
    inline QCStringList outputFormatList() const { return m_outputFormats; }

    InputFormat*  inputFormat(const QCString& name);
    OutputFormat* outputFormat(const QCString& name);
    
private:
    QCStringList m_inputFormats;
    QCStringList m_outputFormats;

    CSVTemplateFactory* m_csvtemplateFactory;

private:
    FormatFactory(const FormatFactory&);
    FormatFactory& operator=(const FormatFactory&);
};

#endif

// End of file

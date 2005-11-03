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

// standard includes
#include <iostream>

// Qt includes
#include <qtextcodec.h>

// KDE includes
#include <kinstance.h>

// KABC includes
#include <kabc/addressee.h>

// local includes
#include "formatfactory.h"
#include "inputformat.h"
#include "outputformat.h"

using std::cerr;
using std::cout;
using std::cin;
using std::endl;

int main(int argc, char** argv)
{
    // check if we have enough commandline parameters
    if (argc != 3)
    {
        cerr << "Wrong number of arguments." << endl;
        cerr << "Usage: converter inputformat outputformat" << endl;
        return -1;
    }

    // create format factory and check if the commandline parameters
    // match format names
    FormatFactory factory;

    InputFormat* input = factory.inputFormat(argv[1]);
    if (input == 0)
    {
        cerr << "Program argument '" << argv[1] << endl;
        cerr << "' does not match any of the known input format names" << endl;
        return -1;
    }

    OutputFormat* output = factory.outputFormat(argv[2]);
    if (output == 0)
    {
        cerr << "Program argument '" << argv[2] << endl;
        cerr << "' does not match any of the known output format names" << endl;

        // don't forget to delete the InputFormat instance we already have
        delete input;

        return -1;
    }

    // create codecs. just use local for this simple example
    QTextCodec* codec = QTextCodec::codecForLocale();
    if (!input->setCodec(codec))
    {
        cerr << "TextCodec '" << codec->name() << endl;
        cerr << "' rejected by input format '" << argv[1] << endl;

        // don't forget to delete format instances
        delete input;
        delete output;

        return -1;
    }

    codec = QTextCodec::codecForLocale();
    if (!output->setCodec(codec))
    {
        cerr << "TextCodec '" << codec->name() << endl;
        cerr << "' rejected by output format '" << argv[2] << endl;

        // don't forget to delete format instances
        delete input;
        delete output;

        return -1;
    }

    // the KInstance instance is needed for the translation calls
    // inside the format implementations.
    // in real programs this is usually the KApplication object
    KInstance instance("converter");

    // as long as there is possibly data on standard input
    // try to read it and convert it
    while (!cin.eof() && !cin.bad())
    {
        KABC::Addressee addressee = input->readAddressee(cin);

        // check if object actually contains anything
        if (addressee.isEmpty())
        {
            cerr << "Failed to read an addressee." << endl;
        }
        else
        {
            output->writeAddressee(addressee, cout);
        }
    }

    delete input;
    delete output;

    // in real programs this is called by QApplication or KApplication
    // respectively
    QTextCodec::deleteAllCodecs();

    return 0;
}

// End of File

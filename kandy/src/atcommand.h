/*
    This file is part of Kandy.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef ATCOMMAND_H
#define ATCOMMAND_H

#include <tqstring.h>
#include <tqstringlist.h>
#include <tqptrlist.h>

class ATParameter {
  public:
    ATParameter();
    ATParameter(const TQString &value,const TQString &name="",
                bool userInput=false);

    void setName(const TQString &name) { mName = name; }
    TQString name() const { return mName; }
    void setValue(const TQString &value) { mValue = value; }
    TQString value() const { return mValue; }
    void setUserInput(bool userInput) { mUserInput = userInput; }
    bool userInput() const { return mUserInput; }

  private:
    TQString mName;
    TQString mValue;
    bool mUserInput;
};

/**
  This class provides an abstraction of an AT command.
*/
// TODO: emit a signal, when the command was executed.
class ATCommand {
  public:
    ATCommand();
    ATCommand(const TQString &cmdString);
    ATCommand(const TQString &cmdName,const TQString &cmdString,
              bool hexOutput=false);
    virtual ~ATCommand();

    void setCmdName(const TQString &);
    TQString cmdName();

    void setCmdString(const TQString &);
    TQString cmdString();

    TQString cmd();

    TQString id();

    void setHexOutput(bool);
    bool hexOutput();

    TQString processOutput(const TQString &);
    TQString processOutput();

    void setResultString(const TQString &);
    TQString resultString();
    TQString resultField(int index);
    TQPtrList<TQStringList> *resultFields();

    void addParameter(ATParameter *);
    void clearParameters();
    TQPtrList<ATParameter> parameters();

    void setParameter(int index,const TQString &value);
    void setParameter(int index,int value);

    void setAutoDelete(bool autoDelete) { mAutoDelete = autoDelete; }
    bool autoDelete() { return mAutoDelete; }

  private:
    void construct();
    void setResultFields(TQString fieldsString);
    void extractParameters();

    TQString mCmdName;
    TQString mCmdString;
    TQString mId;
    bool mHexOutput;

    TQString mResultString;
    TQPtrList<TQStringList> mResultFieldsList;

    TQPtrList<ATParameter> mParameters;

    bool mAutoDelete;
};

#endif

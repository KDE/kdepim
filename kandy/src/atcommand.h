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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef ATCOMMAND_H
#define ATCOMMAND_H

#include <qstring.h>
#include <qstringlist.h>
#include <qptrlist.h>

class ATParameter {
  public:
    ATParameter();
    ATParameter(const QString &value,const QString &name="",
                bool userInput=false);

    void setName(const QString &name) { mName = name; }
    QString name() const { return mName; }
    void setValue(const QString &value) { mValue = value; }
    QString value() const { return mValue; }
    void setUserInput(bool userInput) { mUserInput = userInput; }
    bool userInput() const { return mUserInput; }

  private:
    QString mName;
    QString mValue;
    bool mUserInput;
};

/**
  This class provides an abstraction of an AT command.
*/
// TODO: emit a signal, when the command was executed.
class ATCommand {
  public:
    ATCommand();
    ATCommand(const QString &cmdString);
    ATCommand(const QString &cmdName,const QString &cmdString,
              bool hexOutput=false);
    virtual ~ATCommand();

    void setCmdName(const QString &);
    QString cmdName();

    void setCmdString(const QString &);
    QString cmdString();

    QString cmd();

    QString id();

    void setHexOutput(bool);
    bool hexOutput();

    QString processOutput(const QString &);
    QString processOutput();

    void setResultString(const QString &);
    QString resultString();
    QString resultField(int index);
    QPtrList<QStringList> *resultFields();

    void addParameter(ATParameter *);
    void clearParameters();
    QPtrList<ATParameter> parameters();

    void setParameter(int index,const QString &value);
    void setParameter(int index,int value);

    void setAutoDelete(bool autoDelete) { mAutoDelete = autoDelete; }
    bool autoDelete() { return mAutoDelete; }

  private:
    void construct();
    void setResultFields(QString fieldsString);
    void extractParameters();

    QString mCmdName;
    QString mCmdString;
    QString mId;
    bool mHexOutput;

    QString mResultString;
    QPtrList<QStringList> mResultFieldsList;

    QPtrList<ATParameter> mParameters;

    bool mAutoDelete;
};

#endif

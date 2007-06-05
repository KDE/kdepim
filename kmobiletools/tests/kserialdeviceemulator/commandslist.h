/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/
#ifndef COMMANDSLIST_H
#define COMMANDSLIST_H

#include <q3valuelist.h>
#include <qstring.h>
/**
	@author Marco Gulino <marco@kmobiletools.org>
*/
class Command {
    public:
        Command() { i_origpos=0; }
        Command(uint origpos, const QString &cmd, const QString &answer)
        { i_origpos=origpos, s_cmd=cmd; s_answer=answer; }
        ~Command() {}
        QString cmd() const { return s_cmd.upper(); }
        QString answer() const { return s_answer; }
        uint origPos() const { return i_origpos; }
        void setPos(uint newpos) { i_origpos=newpos; }
        void setCmd(const QString &cmd) { s_cmd=cmd; }
        void setAnswer(const QString &answer) { s_answer=answer; }
        bool isNull() { return ! bool(s_answer.length() && s_cmd.length() ); }
        bool operator ==(Command compcmd);
    private:
        QString s_cmd;
        QString s_answer;
        uint i_origpos;
};

class CommandsList : public Q3ValueList<Command>
{
public:
    CommandsList();
    ~CommandsList();
    static CommandsList *instance() { return p_instance; }
    void loadFile(const QString &filen);
    Command searchCmd(const QString &cmd);
    bool hasSMSSlots() { return b_hassmsslots; }
    private:
        static CommandsList *p_instance;
        bool b_hassmsslots;

};

#endif

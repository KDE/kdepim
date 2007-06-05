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
#include "commandslist.h"
#include <qfile.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3TextStream>
#include <kdebug.h>

CommandsList *CommandsList::p_instance=0;

CommandsList::CommandsList()
    : Q3ValueList<Command>()
{
    p_instance=this;
}


CommandsList::~CommandsList()
{
}

bool Command::operator ==( Command compcmd)
{
    return (compcmd.origPos()==i_origpos && compcmd.cmd()== s_cmd && compcmd.answer()== s_answer);
}


/*!
    \fn CommandsList::loadFile(const QString &file)
 */
void CommandsList::loadFile(const QString &filen)
{
    int status=0; // 0=nothing, 1=command, 2=answer
    QString line, cmd, answer;
    QFile file(filen);
    if (! file.exists()) return;
    clear();
    b_hassmsslots=true;
    file.open(QIODevice::ReadOnly);
    uint i=0;
    Q3TextStream stream(&file);
    while ( !stream.atEnd() ) {
        line=stream.readLine();
        if(line.left(1) == "#") {
        // Command-comment
            if (line.left(2) == "##") { // Command directive
                if (line.contains("NOSMSSLOTS")) {
                    b_hassmsslots=false;
                    kDebug() << "Setting NO sms slots\n";
                }
            }
            continue;
        }
        if(line.left(3) == ">>>")
        {
            if(cmd.length() && answer.length())
            {
//                 kDebug() << "Found command set: cmd==" << cmd << ";; answer==" << answer << ";;\n";
                append( Command(i,cmd, answer));
                cmd.clear();
                answer.clear();
                i++;
            }
            status=1;
            cmd=line.mid(3);
        } else if(line.left(3) == "<<<" || status==2)
        {
            if(status==2)
            {
                answer+='\n' + line;
            } else
            {
                status=2;
                answer=line.mid(3);
            }
        }
    }
    file.close();
    /// @todo implement me
}


/*!
    \fn CommandsList::searchCmd(const QString &cmd)
 */
Command CommandsList::searchCmd(const QString &cmd)
{
    for( Q3ValueList<Command>::Iterator it=begin(); it!=end() ; ++it ){
        if((*it).cmd()==cmd) return *it;
    }
    return Command();
}

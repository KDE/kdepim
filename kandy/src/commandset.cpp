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

#include <qdom.h>
#include <qfile.h>
#include <qtextstream.h>

#include <kdebug.h>

#include "atcommand.h"

#include "commandset.h"

CommandSet::CommandSet()
{
  mList.setAutoDelete(true);
}

CommandSet::~CommandSet()
{
}

void CommandSet::addCommand(ATCommand *command)
{
  mList.append(command);
}

void CommandSet::deleteCommand(ATCommand *command)
{
  mList.removeRef(command);
}

bool CommandSet::loadFile(const QString& filename)
{
//  kdDebug() << "CommandSet::loadFile(): " << filename << endl;

  QDomDocument doc("Kandy");
  QFile f(filename);
  if (!f.open(IO_ReadOnly))
    return false;
  if (!doc.setContent(&f)) {
    f.close();
    return false;
  }
  f.close();

  QDomNodeList commands = doc.elementsByTagName("command");
  for(uint i=0;i<commands.count();++i) {
    QDomElement c = commands.item(i).toElement();
    if (!c.isNull()) {
      ATCommand *cmd = new ATCommand;
      loadCommand(cmd,&c);
      addCommand(cmd);
    }
  }
  
  return true;
}

bool CommandSet::saveFile(const QString& filename)
{
  kdDebug() << "CommandSet::saveFile(): " << filename << endl;

  QDomDocument doc("Kandy");
  QDomElement set = doc.createElement("commandset");
  doc.appendChild(set);

  for(uint i=0; i<mList.count();++i) {
    saveCommand(mList.at(i),&doc,&set);
  }
  
  QFile xmlfile(filename);
  if (!xmlfile.open(IO_WriteOnly)) {
    kdDebug() << "Error opening file for write." << endl;
    return false;
  }
  QTextStream ts(&xmlfile);
  doc.documentElement().save(ts,2);
  xmlfile.close();

  return true;
}

void CommandSet::clear()
{
  mList.clear();
}

void CommandSet::loadCommand(ATCommand *command,QDomElement *c)
{
  command->setCmdName(c->attribute("name","unknown"));
  command->setCmdString(c->attribute("string","at"));
  command->setHexOutput(c->attribute("hexoutput","n") == "y");

  QDomNode n = c->firstChild();
  while(!n.isNull()) {
    QDomElement e = n.toElement();
    if (!e.isNull()) {
      ATParameter *p = new ATParameter;
      p->setName(e.attribute("name","unnamed"));
      p->setValue(e.attribute("value","0"));
      p->setUserInput(e.attribute("userinput","n") == "y");

      command->addParameter(p);
    }
    n = n.nextSibling();
  }
}

void CommandSet::saveCommand(ATCommand *command,QDomDocument *doc,
                             QDomElement *parent)
{
  QDomElement c = doc->createElement("command");
  c.setAttribute("name",command->cmdName());
  c.setAttribute("string",command->cmdString());
  c.setAttribute("hexoutput",command->hexOutput() ? "y" : "n");
  parent->appendChild(c);
  
  QPtrList<ATParameter> paras = command->parameters();
  for(uint i=0;i<paras.count();++i) {
    saveParameter(paras.at(i),doc,&c);
  }
}

void CommandSet::saveParameter(ATParameter *p, QDomDocument *doc,
                               QDomElement *parent)
{
  QDomElement e = doc->createElement("parameter");
  e.setAttribute("name",p->name());
  e.setAttribute("value",p->value());
  e.setAttribute("userinput",p->userInput() ? "y" : "n");
  parent->appendChild(e);
}

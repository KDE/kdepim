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

#include <kdebug.h>

#include "atcommand.h"

#include "commanditem.h"

CommandItem::CommandItem(QListView *listView,ATCommand *command)
  : QListViewItem(listView)
{
  mCommand = command;
  
  setItemText();
}

CommandItem::~CommandItem()
{
}

ATCommand *CommandItem::command()
{
  return mCommand;
}

void CommandItem::load(QDomElement *c)
{
  mCommand->setCmdName(c->attribute("name","unknown"));
  mCommand->setCmdString(c->attribute("string","at"));
  mCommand->setHexOutput(c->attribute("hexoutput","n") == "y");

  QDomNode n = c->firstChild();
  while(!n.isNull()) {
    QDomElement e = n.toElement();
    if (!e.isNull()) {
      ATParameter *p = new ATParameter;
      p->setName(e.attribute("name","unnamed"));
      p->setValue(e.attribute("value","0"));
      p->setUserInput(e.attribute("userinput","n") == "y");

      mCommand->addParameter(p);
    }
    n = n.nextSibling();
  }

  setItemText();
}

void CommandItem::save(QDomDocument *doc,QDomElement *parent)
{
  QDomElement c = doc->createElement("command");
  c.setAttribute("name",mCommand->cmdName());
  c.setAttribute("string",mCommand->cmdString());
  c.setAttribute("hexoutput",mCommand->hexOutput() ? "y" : "n");
  parent->appendChild(c);
  
  QPtrList<ATParameter> paras = mCommand->parameters();
  for(uint i=0;i<paras.count();++i) {
    saveParameter(paras.at(i),doc,&c);
  }
}

void CommandItem::saveParameter(ATParameter *p, QDomDocument *doc,
                                QDomElement *parent)
{
  QDomElement e = doc->createElement("parameter");
  e.setAttribute("name",p->name());
  e.setAttribute("value",p->value());
  e.setAttribute("userinput",p->userInput() ? "y" : "n");
  parent->appendChild(e);
}

void CommandItem::setItemText()
{
  setText(0,mCommand->cmdName());
  setText(1,mCommand->cmdString());
  setText(2,mCommand->hexOutput() ? "y" : "n");
}

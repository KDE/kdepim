// $Id$

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
  delete mCommand;
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
  
  QList<ATParameter> paras = mCommand->parameters();
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

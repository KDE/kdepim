// $Id$

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
  
  QList<ATParameter> paras = command->parameters();
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

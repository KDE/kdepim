/*
    This file is part of Blogilo, A KDE Blogging Client

    It is a modified version of "utilities.cpp" from
    FlashQard project.

    Copyright (C) 2008-2009 Shahab <shahab@flashqard-project.org>
    Copyright (C) 2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2010 Golnaz Nilieh <g382nilieh@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.


    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/
*/

#include <QRegExp>
#include <QToolBar>
#include <QMessageBox>
#include <QApplication>
#include <QTextDocument>
#include <QTextCursor>
#include <QtWebKit>
#include "utilities.h"
// #include "Settings.h"

HtmlParser* HtmlParser::_htmlParser = 0;

HtmlParser* HtmlParser::htmlParser()
{
   if (_htmlParser)
      return _htmlParser;
   else
      return _htmlParser = new HtmlParser;
}

HtmlParser::HtmlParser()
{
   webPage = new QWebPage();
}

QString HtmlParser::htmlToPlainText(const QString &html)
{
   webPage->mainFrame()->setHtml(html);
   return webPage->mainFrame()->toPlainText();

   
//    int tagCount=0;
//    int iterator=0;
//    int tagPosition=0;
//    int tagLength=0;
//    QString tagName;
//    QString plainText;

//    if (!findTag(html, iterator, "body", tagPosition, tagLength))
//       iterator = 0;
//    else
//       iterator = tagPosition + tagLength;

//    bool firstP = true;
//    while (findNextTag(html, iterator, tagName, tagPosition, tagLength))
//    {
//       tagCount++;
//       QString tmpText = html.mid(iterator, tagPosition-iterator);
//       bool spaceAtBeginning = false;
//       bool spaceAtEnd = false;
//       if (tmpText.size())
//       {
//          spaceAtBeginning = tmpText[0].isSpace();
//          spaceAtEnd = tmpText[tmpText.size()-1].isSpace();
//       }

//       tmpText = tmpText.simplified();
//       if (spaceAtBeginning)
//          tmpText.insert(0, ' ');
//       if (spaceAtEnd)
//          tmpText.append(' ');
      
//       plainText += tmpText;
//       if (tagName.compare("p", Qt::CaseInsensitive) == 0 && !firstP)
//          plainText += "\n";
//       else if (tagName.compare("br", Qt::CaseInsensitive) == 0)
//          plainText += "\n";

//       firstP = false;
//       iterator = tagPosition+tagLength;
//    }
   
//    if (tagCount==0)
//       plainText = html;

//    plainText.replace("&lt;", "<", Qt::CaseInsensitive);
//    plainText.replace("&gt;", ">", Qt::CaseInsensitive);
//    return plainText;
}

bool HtmlParser::findNextTag(const QString &html, int iterator, QString &tagName, int &tagPosition, int &tagLength) const
{
   QRegExp regExp("<(/?)\\s*(\\w+|!--)[^>]*>", Qt::CaseInsensitive);
   tagPosition = regExp.indexIn(html, iterator);
//   Due to a different behaviour of qt 4.5 from previous versions the line below should not be used
//   (replaced by the line above)
//   tagPosition = html.indexOf(regExp, iterator);

   if (tagPosition == -1)
      return false;

   tagLength = regExp.matchedLength();
   tagName = regExp.cap(1)+regExp.cap(2);
   return true;
}

bool HtmlParser::findTag(const QString &html, int iterator, const QString &tagName, int &tagPosition, int &tagLength) const
{
   QString currentTagName;
   while (findNextTag(html, iterator, currentTagName, tagPosition, tagLength))
   {
      if (currentTagName.compare(tagName, Qt::CaseInsensitive) == 0)
         return true;
      iterator = tagPosition + tagLength;
   }
   return false;
}

bool HtmlParser::setTagAttribute(QString &html, int &iterator, const QString &tagName,
                                    const QString &attributeName, const QString &attributeValue)
{
   int tagPosition=0;
   int tagLength=0;
   
   if (!findTag(html, iterator, tagName, tagPosition, tagLength))
      return false;

   QString completeTag = html.mid(tagPosition, tagLength);

   //name attributeName = "attributeValue"
   QRegExp regExp("\\W(" + attributeName + "\\s*=\\s*)\"([^\"]*)\"", Qt::CaseInsensitive);
   if (regExp.indexIn(completeTag) != -1)
   {
      completeTag.remove(regExp.pos(2), regExp.cap(2).length());
      completeTag.insert(regExp.pos(2), attributeValue);
   }
   else
      completeTag.insert(completeTag.length()-1, " " + attributeName + "=\"" + attributeValue + "\"");

   html.remove(tagPosition, tagLength);
   html.insert(tagPosition, completeTag);

   iterator += completeTag.length();
   return true;
}

bool HtmlParser::getTagAttribute(const QString &html, int &iterator, const QString &tagName,
                                    const QString &attributeName, QString &attributeValue)
{
   int tagPosition=0;
   int tagLength=0;
   
   if (!findTag(html, iterator, tagName, tagPosition, tagLength))
      return false;

   QString completeTag = html.mid(tagPosition, tagLength);

   //tagName attributeName = "attributeValue"
   QRegExp regExp("\\W(" + attributeName + "\\s*=\\s*)\"([^\"]*)\"", Qt::CaseInsensitive);
   if (regExp.indexIn(completeTag) != -1)
      attributeValue = regExp.cap(2);
   else
      attributeValue = QString();

   iterator += completeTag.length();
   return true;
}


bool HtmlParser::setTagStyleAttribute(QString &html, int iterator, const QString &tagName,
                                         const QString &attributeName, const QString &attributeValue)
{
   int tagPosition=0;
   int tagLength=0;
   
   if (!findTag(html, iterator, tagName, tagPosition, tagLength))
      return false;

   QString completeTag = html.mid(tagPosition, tagLength);

   QRegExp regExp("\\W(style\\s*=\\s*)\"([^\"]*)\"", Qt::CaseInsensitive);
   if (regExp.indexIn(completeTag) != -1)
   {
      QRegExp innerRegExp("\\W" + attributeName + "\\s*:\\s*([^;$]*)[;$]", Qt::CaseInsensitive);
      if (innerRegExp.indexIn(regExp.cap(2)) != -1)
         {
            QString styleAttributes = regExp.cap(2);
            styleAttributes.remove(innerRegExp.pos(1), innerRegExp.cap(1).length());
            styleAttributes.insert(innerRegExp.pos(1), attributeValue);
            completeTag.remove(regExp.pos(2), regExp.cap(2).length());
            completeTag.insert(regExp.pos(2), styleAttributes);
         }
         else
         {
            completeTag.insert(regExp.pos(2), attributeName + ":" + attributeValue + ";");
         }
   }
   else
      completeTag.insert(completeTag.length()-1, " style=\"" + attributeName + ":" + attributeValue + ";\"");

   html.remove(tagPosition, tagLength);
   html.insert(tagPosition, completeTag);

   return true;
}

bool HtmlParser::contains(const QString &html, const QString &string, bool isRegExp) const
{
   bool contains = false;
   QRegExp regExp = isRegExp ? QRegExp(string) : QRegExp(string, Qt::CaseInsensitive, QRegExp::Wildcard);
   contains = html.contains(regExp);
   if (contains)
   {
      QTextDocument document;
      document.setHtml(html);
      contains = !document.find(regExp).isNull();
   }
   return contains;
}

QString HtmlParser::contentsOfTag(const QString &html, int iterator, const QString &tagName) const
{
   int tagPosition=0;
   int tagLength=0;
   int contentsPosition=0;
   
   if (!findTag(html, iterator, tagName, tagPosition, tagLength))
      return QString();

   contentsPosition = tagPosition+tagLength;

   if (!findTag(html, iterator, "/"+tagName, tagPosition, tagLength))
      return QString();

   return html.mid(contentsPosition, tagPosition-contentsPosition);
}

Random* Random::_random = 0;

Random* Random::random()
{
   if (_random)
      return _random;
   else
      return (_random = new Random);
}

Random::Random()
{
   srand(time(0));
}

int Random::getARandomNumber(int min, int max)
{
   return rand()%(max-min+1)+min;
}

QString Random::getARandomName(int maxLength)
{
   QString characters ("abcdefghijklmnopqrstuvwxyz0123456789");
   QString name;
   for (int i=0; i<maxLength; i++)
      name.append(characters[getARandomNumber(0, characters.size()-1)]);
   return name;
}


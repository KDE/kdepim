/*
    utilities.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2000 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include <qframe.h>
#include <qlayout.h>

#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kdialogbase.h>

#include "knlistbox.h"
#include "knglobals.h"
#include "utilities.h"


// **** keyboard selection dialog *********************************************

int selectDialog(QWidget *parent, const QString &caption, const QStringList &options, int initialValue)
{
  KDialogBase *dlg=new KDialogBase(KDialogBase::Plain, caption, KDialogBase::Ok|KDialogBase::Cancel,
                                   KDialogBase::Ok, parent);
  QFrame *page = dlg->plainPage();
  QHBoxLayout *pageL = new QHBoxLayout(page,8,5);

  KNDialogListBox *list = new KNDialogListBox(page);
  pageL->addWidget(list);

  QString s;
  for ( QStringList::ConstIterator it = options.begin(); it != options.end(); ++it ) {
    s = (*it);
    s.replace(QRegExp("&"),"");   // remove accelerators
    list->insertItem(s);
  }

  list->setCurrentItem(initialValue);
  list->setFocus();
  restoreWindowSize("selectBox", dlg, QSize(247,174));

  int ret;
  if (dlg->exec())
    ret = list->currentItem();
  else
    ret = -1;

  saveWindowSize("selectBox", dlg->size());
  delete dlg;
  return ret;
}

// **** window geometry managing *********************************************

void saveWindowSize(const QString &name, const QSize &s)
{
  KConfig *c=KGlobal::config();
  c->setGroup("WINDOW_SIZES");
  c->writeEntry(name, s); 
}


void restoreWindowSize(const QString &name, QWidget *d, const QSize &defaultSize)
{
  KConfig *c=KGlobal::config();
  c->setGroup("WINDOW_SIZES");
  
  QSize s=c->readSizeEntry(name,&defaultSize);
  
  if(s.isValid()) d->resize(s); 
}

// **** scramble password strings **********************************************

const QString encryptStr(const QString& aStr)
{
  uint i,val,len = aStr.length();
  QCString result;

  for (i=0; i<len; i++)
  {
    val = aStr[i] - ' ';
    val = (255-' ') - val;
    result += (char)(val + ' ');
  }

  return result;
}


const QString decryptStr(const QString& aStr)
{
  return encryptStr(aStr);
}

// **** rot13 *******************************************************************

QString rot13(const QString &s)
{
  QString r(s);

  for (int i=0; (uint)i<r.length(); i++) {
    if ( r[i] >= QChar('A') && r[i] <= QChar('M') ||
         r[i] >= QChar('a') && r[i] <= QChar('m') )
         r[i] = (char)((int)QChar(r[i]) + 13);
    else
      if  ( r[i] >= QChar('N') && r[i] <= QChar('Z') ||
            r[i] >= QChar('n') && r[i] <= QChar('z') )
        r[i] = (char)((int)QChar(r[i]) - 13);
  }  	

  return r;
}

// **** us-ascii check **********************************************************

bool isUsAscii(const QString &s)
{
  for (uint i=0; i<s.length(); i++)
    if (s.at(i).latin1()<=0)    // c==0: non-latin1, c<0: non-us-ascii
      return false;

  return true;
}

// **** text rewraping *********************************************************

int findBreakPos(const QString &text, int start)
{
  int i;
  for(i=start;i>=0;i--)
    if(text[i].isSpace())
      break;
  if(i>0)
    return i;
  for(i=start;i<(int)text.length();i++)   // ok, the line is to long
    if(text[i].isSpace())
      break;
  return i;
}


void appendTextWPrefix(QString &result, const QString &text, int wrapAt, const QString &prefix)
{
  QString txt=text;
  int breakPos;

  while(!txt.isEmpty()) {

    if((int)(prefix.length()+txt.length()) > wrapAt) {
      breakPos=findBreakPos(txt,wrapAt-prefix.length());
      result+=(prefix+txt.left(breakPos)+"\n");
      txt.remove(0,breakPos+1);
    } else {
      result+=(prefix+txt+"\n");
      txt=QString::null;
    }
  }
}


QString rewrapStringList(QStringList text, int wrapAt, QChar quoteChar, bool stopAtSig, bool alwaysSpace)
{
  QString quoted, lastPrefix, thisPrefix, leftover, thisLine;
  int breakPos;

  for(QStringList::Iterator line=text.begin(); line!=text.end(); ++line) {

    if(stopAtSig && (*line)=="-- ")
      break;

    thisLine=(*line);
    if (!alwaysSpace && (thisLine[0]==quoteChar))
      thisLine.prepend(quoteChar);  // second quote level without space
    else
      thisLine.prepend(quoteChar+' ');

    thisPrefix=QString::null;
    QChar c;
    for(int idx=0; idx<(int)(thisLine.length()); idx++) {
      c=thisLine.at(idx);
      if( (c==' ') ||
          (c==quoteChar) || (c=='>') ||(c=='|') || (c==':') || (c=='#') || (c=='[') || (c=='{'))
        thisPrefix.append(c);
      else
        break;
    }

    thisLine.remove(0,thisPrefix.length());
    thisLine = thisLine.stripWhiteSpace();

    if(!leftover.isEmpty()) {   // don't break paragraphs, tables and quote levels
      if(thisLine.isEmpty() || (thisPrefix!=lastPrefix) || thisLine.contains("  ") || thisLine.contains('\t'))
        appendTextWPrefix(quoted, leftover, wrapAt, lastPrefix);
      else
        thisLine.prepend(leftover+" ");
      leftover=QString::null;
    }

    if((int)(thisPrefix.length()+thisLine.length()) > wrapAt) {
      breakPos=findBreakPos(thisLine,wrapAt-thisPrefix.length());
      if(breakPos < (int)(thisLine.length())) {
        leftover=thisLine.right(thisLine.length()-breakPos-1);
        thisLine.truncate(breakPos);
      }
    }

    quoted+=thisPrefix+thisLine+"\n";
    lastPrefix=thisPrefix;
  }

  if (!leftover.isEmpty())
    appendTextWPrefix(quoted, leftover, wrapAt, lastPrefix);

  return quoted;
}

// **** misc. message-boxes **********************************************************

void displayInternalFileError(QWidget *w)
{
  KMessageBox::error((w!=0)? w : knGlobals.topWidget, i18n("Unable to load/save configuration!\nWrong permissions on home directory?\nYou should close KNode now, to avoid data loss!"));
}


void displayExternalFileError(QWidget *w)
{
  KMessageBox::error((w!=0)? w : knGlobals.topWidget, i18n("Unable to load/save file!"));
}


void displayRemoteFileError(QWidget *w)
{
  KMessageBox::error((w!=0)? w : knGlobals.topWidget, i18n("Unable to save remote file!"));
}


void displayTempFileError(QWidget *w)
{
  KMessageBox::error((w!=0)? w : knGlobals.topWidget, i18n("Unable to create temporary file!"));
}

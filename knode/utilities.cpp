/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include <tqlayout.h>
#include <tqregexp.h>
#include <tqapplication.h>
#include <tqcursor.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <kio/netaccess.h>
#include <ktempfile.h>
#include <kfiledialog.h>

#include "knwidgets.h"
#include "knglobals.h"
#include "utilities.h"



//================================================================================

KNFile::KNFile(const TQString& fname)
 : TQFile(fname), filePos(0), readBytes(0)
{
  buffer.resize(512);
  dataPtr=buffer.data();
  dataPtr[0]='\0';
}


KNFile::~KNFile()
{
}


const TQCString& KNFile::readLine()
{
  filePos=at();
  readBytes=TQFile::readLine(dataPtr, buffer.size()-1);
  if(readBytes!=-1) {
    while ((dataPtr[readBytes-1]!='\n')&&(static_cast<uint>(readBytes+2)==buffer.size())) {  // don't get tricked by files without newline
      at(filePos);
      if (!increaseBuffer() ||
         (readBytes=TQFile::readLine(dataPtr, buffer.size()-1))==-1) {
        readBytes=1;
        break;
      }
    }
  } else
    readBytes=1;

  dataPtr[readBytes-1] = '\0';
  return buffer;
}


const TQCString& KNFile::readLineWnewLine()
{
  filePos=at();
  readBytes=TQFile::readLine(dataPtr, buffer.size()-1);
  if(readBytes!=-1) {
    while ((dataPtr[readBytes-1]!='\n')&&(static_cast<uint>(readBytes+2)==buffer.size())) {  // don't get tricked by files without newline
      at(filePos);
      if (!increaseBuffer() ||
         (readBytes=TQFile::readLine(dataPtr, buffer.size()-1))==-1) {
        dataPtr[0] = '\0';
        break;
      }
    }
  }
  else dataPtr[0] = '\0';

  return buffer;
}


int KNFile::findString(const char *s)
{
  TQCString searchBuffer;
  searchBuffer.resize(2048);
  char *buffPtr = searchBuffer.data(), *pos;
  int readBytes, currentFilePos;

  while (!atEnd()) {
    currentFilePos = at();
    readBytes = readBlock(buffPtr, 2047);
    if (readBytes == -1)
      return -1;
    else
      buffPtr[readBytes] = 0;       // terminate string

    pos = strstr(buffPtr,s);
    if (pos == 0) {
      if (!atEnd())
        at(at()-strlen(s));
      else
        return -1;
    } else {
      return currentFilePos + (pos-buffPtr);
    }
  }

  return -1;
}


bool KNFile::increaseBuffer()
{
  if(buffer.resize(2*buffer.size())) {;
    dataPtr=buffer.data();
    dataPtr[0]='\0';
    kdDebug(5003) << "KNFile::increaseBuffer() : buffer doubled" << endl;
    return true;
  }
  else return false;
}


//===============================================================================

TQString KNSaveHelper::lastPath;

KNSaveHelper::KNSaveHelper(TQString saveName, TQWidget *parent)
  : p_arent(parent), s_aveName(saveName), file(0), tmpFile(0)
{
}


KNSaveHelper::~KNSaveHelper()
{
  if (file) {       // local filesystem, just close the file
    delete file;
  } else
    if (tmpFile) {      // network location, initiate transaction
      tmpFile->close();
      if (KIO::NetAccess::upload(tmpFile->name(),url, 0) == false)
        KNHelper::displayRemoteFileError();
      tmpFile->unlink();   // delete temp file
      delete tmpFile;
    }
}


TQFile* KNSaveHelper::getFile(const TQString &dialogTitle)
{
  url = KFileDialog::getSaveURL(lastPath + s_aveName, TQString::null, p_arent, dialogTitle);

  if (url.isEmpty())
    return 0;

  lastPath = url.upURL().url();

  if (url.isLocalFile()) {
    if (TQFileInfo(url.path()).exists() &&
        (KMessageBox::warningContinueCancel(knGlobals.topWidget,
                                            i18n("<qt>A file named <b>%1</b> already exists.<br>Do you want to replace it?</qt>").arg(url.path()),
                                            dialogTitle, i18n("&Replace")) != KMessageBox::Continue)) {
      return 0;
    }

    file = new TQFile(url.path());
    if(!file->open(IO_WriteOnly)) {
      KNHelper::displayExternalFileError();
      delete file;
      file = 0;
    }
    return file;
  } else {
    tmpFile = new KTempFile();
    if (tmpFile->status()!=0) {
      KNHelper::displayTempFileError();
      delete tmpFile;
      tmpFile = 0;
      return 0;
    }
    return tmpFile->file();
  }
}


//===============================================================================

TQString KNLoadHelper::l_astPath;

KNLoadHelper::KNLoadHelper(TQWidget *parent)
  : p_arent(parent), f_ile(0)
{
}


KNLoadHelper::~KNLoadHelper()
{
  delete f_ile;
  if (!t_empName.isEmpty())
    KIO::NetAccess::removeTempFile(t_empName);
}


KNFile* KNLoadHelper::getFile( const TQString &dialogTitle )
{
  if (f_ile)
    return f_ile;

  KURL url = KFileDialog::getOpenURL(l_astPath,TQString::null,p_arent,dialogTitle);

  if (url.isEmpty())
    return 0;

  l_astPath = url.url(-1);
  l_astPath.truncate(l_astPath.length()-url.fileName().length());

  return setURL(url);
}


KNFile* KNLoadHelper::setURL(KURL url)
{
  if (f_ile)
    return f_ile;

  u_rl = url;

  if (u_rl.isEmpty())
    return 0;

  TQString fileName;
  if (!u_rl.isLocalFile()) {
    if (KIO::NetAccess::download(u_rl, t_empName, 0))
      fileName = t_empName;
  } else
    fileName = u_rl.path();

  if (fileName.isEmpty())
    return 0;

  f_ile = new KNFile(fileName);
  if(!f_ile->open(IO_ReadOnly)) {
    KNHelper::displayExternalFileError();
    delete f_ile;
    f_ile = 0;
  }
  return f_ile;
}


//===============================================================================


// **** keyboard selection dialog *********************************************
int KNHelper::selectDialog(TQWidget *parent, const TQString &caption, const TQStringList &options, int initialValue)
{
  KDialogBase *dlg=new KDialogBase(KDialogBase::Plain, caption, KDialogBase::Ok|KDialogBase::Cancel,
                                   KDialogBase::Ok, parent);
  TQFrame *page = dlg->plainPage();
  TQHBoxLayout *pageL = new TQHBoxLayout(page,8,5);

  KNDialogListBox *list = new KNDialogListBox(true, page);
  pageL->addWidget(list);

  TQString s;
  for ( TQStringList::ConstIterator it = options.begin(); it != options.end(); ++it ) {
    s = (*it);
    s.replace(TQRegExp("&"),"");   // remove accelerators
    list->insertItem(s);
  }

  list->setCurrentItem(initialValue);
  list->setFocus();
  restoreWindowSize("selectBox", dlg, TQSize(247,174));

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

void KNHelper::saveWindowSize(const TQString &name, const TQSize &s)
{
  KConfig *c=knGlobals.config();
  c->setGroup("WINDOW_SIZES");
  c->writeEntry(name, s);
}


void KNHelper::restoreWindowSize(const TQString &name, TQWidget *d, const TQSize &defaultSize)
{
  KConfig *c=knGlobals.config();
  c->setGroup("WINDOW_SIZES");

  TQSize s=c->readSizeEntry(name,&defaultSize);

  if(s.isValid()) {
    TQRect max = KGlobalSettings::desktopGeometry(TQCursor::pos());
    if ( s.width() > max.width() ) s.setWidth( max.width()-5 );
    if ( s.height() > max.height() ) s.setHeight( max.height()-5 );
    d->resize(s);
  }
}

// **** scramble password strings **********************************************

const TQString KNHelper::encryptStr(const TQString& aStr)
{
  uint i,val,len = aStr.length();
  TQCString result;

  for (i=0; i<len; i++)
  {
    val = aStr[i] - ' ';
    val = (255-' ') - val;
    result += (char)(val + ' ');
  }

  return result;
}


const TQString KNHelper::decryptStr(const TQString& aStr)
{
  return encryptStr(aStr);
}

// **** rot13 *******************************************************************

TQString KNHelper::rot13(const TQString &s)
{
  TQString r(s);

  for (int i=0; (uint)i<r.length(); i++) {
    if ( r[i] >= TQChar('A') && r[i] <= TQChar('M') ||
         r[i] >= TQChar('a') && r[i] <= TQChar('m') )
         r[i] = (char)((int)TQChar(r[i]) + 13);
    else
      if  ( r[i] >= TQChar('N') && r[i] <= TQChar('Z') ||
            r[i] >= TQChar('n') && r[i] <= TQChar('z') )
        r[i] = (char)((int)TQChar(r[i]) - 13);
  }

  return r;
}

// **** text rewraping *********************************************************

int findBreakPos(const TQString &text, int start)
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


void appendTextWPrefix(TQString &result, const TQString &text, int wrapAt, const TQString &prefix)
{
  TQString txt=text;
  int breakPos;

  while(!txt.isEmpty()) {

    if((int)(prefix.length()+txt.length()) > wrapAt) {
      breakPos=findBreakPos(txt,wrapAt-prefix.length());
      result+=(prefix+txt.left(breakPos)+"\n");
      txt.remove(0,breakPos+1);
    } else {
      result+=(prefix+txt+"\n");
      txt=TQString::null;
    }
  }
}


TQString KNHelper::rewrapStringList(TQStringList text, int wrapAt, TQChar quoteChar, bool stopAtSig, bool alwaysSpace)
{
  TQString quoted, lastPrefix, thisPrefix, leftover, thisLine;
  int breakPos;

  for(TQStringList::Iterator line=text.begin(); line!=text.end(); ++line) {

    if(stopAtSig && (*line)=="-- ")
      break;

    thisLine=(*line);
    if (!alwaysSpace && (thisLine[0]==quoteChar))
      thisLine.prepend(quoteChar);  // second quote level without space
    else
      thisLine.prepend(quoteChar+' ');

    thisPrefix=TQString::null;
    TQChar c;
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
      leftover=TQString::null;
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

void KNHelper::displayInternalFileError(TQWidget *w)
{
  KMessageBox::error((w!=0)? w : knGlobals.topWidget, i18n("Unable to load/save configuration.\nWrong permissions on home folder?\nYou should close KNode now to avoid data loss."));
}


void KNHelper::displayExternalFileError(TQWidget *w)
{
  KMessageBox::error((w!=0)? w : knGlobals.topWidget, i18n("Unable to load/save file."));
}


void KNHelper::displayRemoteFileError(TQWidget *w)
{
  KMessageBox::error((w!=0)? w : knGlobals.topWidget, i18n("Unable to save remote file."));
}


void KNHelper::displayTempFileError(TQWidget *w)
{
  KMessageBox::error((w!=0)? w : knGlobals.topWidget, i18n("Unable to create temporary file."));
}

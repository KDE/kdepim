/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2006 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include <QByteArray>
#include <QListWidget>
#include <QRegExp>
#include <QCursor>

#include <QHBoxLayout>

#include <klocale.h>
#include <kmessagebox.h>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <kio/netaccess.h>
#include <ktemporaryfile.h>
#include <kfiledialog.h>
#include <kdialog.h>

#include "knglobals.h"
#include "utilities.h"



QString KNSaveHelper::lastPath;

KNSaveHelper::KNSaveHelper(QString saveName, QWidget *parent)
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
      if (KIO::NetAccess::upload(tmpFile->fileName(),url, 0) == false)
        KNHelper::displayRemoteFileError();
      delete tmpFile;
    }
}


QFile* KNSaveHelper::getFile(const QString &dialogTitle)
{
  url = KFileDialog::getSaveUrl(QString(lastPath + s_aveName), QString(), p_arent, dialogTitle);

  if (url.isEmpty())
    return 0;

  lastPath = url.upUrl().url();

  if (url.isLocalFile()) {
    if (QFileInfo(url.toLocalFile()).exists() &&
        (KMessageBox::warningContinueCancel(knGlobals.topWidget,
                                            i18n("<qt>A file named <b>%1</b> already exists.<br />Do you want to replace it?</qt>", url.path()),
                                            dialogTitle, KGuiItem(i18n("&Replace"))) != KMessageBox::Continue)) {
      return 0;
    }

    file = new QFile(url.toLocalFile());
    if(!file->open(QIODevice::WriteOnly)) {
      KNHelper::displayExternalFileError();
      delete file;
      file = 0;
    }
    return file;
  } else {
    tmpFile = new KTemporaryFile();
    if (!tmpFile->open()) {
      KNHelper::displayTempFileError();
      delete tmpFile;
      tmpFile = 0;
      return 0;
    }
    return tmpFile;
  }
}


//===============================================================================

KUrl KNLoadHelper::l_astPath;

KNLoadHelper::KNLoadHelper(QWidget *parent)
  : p_arent(parent), f_ile(0)
{
}


KNLoadHelper::~KNLoadHelper()
{
  delete f_ile;
  if (!t_empName.isEmpty())
    KIO::NetAccess::removeTempFile(t_empName);
}


QFile* KNLoadHelper::getFile( const QString &dialogTitle )
{
  if (f_ile)
    return f_ile;

  KUrl url = KFileDialog::getOpenUrl( l_astPath, QString(), p_arent, dialogTitle );

  if (url.isEmpty())
    return 0;

  l_astPath = url;

  return setURL(url);
}


QFile* KNLoadHelper::setURL(const KUrl& url)
{
  if (f_ile)
    return f_ile;

  u_rl = url;

  if (u_rl.isEmpty())
    return 0;

  QString fileName;
  if (!u_rl.isLocalFile()) {
    if (KIO::NetAccess::download(u_rl, t_empName, 0))
      fileName = t_empName;
  } else
    fileName = u_rl.toLocalFile();

  if (fileName.isEmpty())
    return 0;

  f_ile = new QFile( fileName );
  if(!f_ile->open(QIODevice::ReadOnly)) {
    KNHelper::displayExternalFileError();
    delete f_ile;
    f_ile = 0;
  }
  return f_ile;
}


//===============================================================================


// **** keyboard selection dialog *********************************************
int KNHelper::selectDialog(QWidget *parent, const QString &caption, const QStringList &options, int initialValue)
{
  KDialog *dlg = new KDialog( parent );
  dlg->setCaption( caption );
  dlg->setButtons( KDialog::Ok|KDialog::Cancel );
  dlg->setDefaultButton( KDialog::Ok );

  QFrame *page = new QFrame( dlg );
  dlg->setMainWidget( page );
  QHBoxLayout *pageL = new QHBoxLayout(page);
  pageL->setSpacing(5);
  pageL->setMargin(8);

  QListWidget *list = new QListWidget( page );
  pageL->addWidget(list);

  QString s;
  for ( QStringList::ConstIterator it = options.begin(); it != options.end(); ++it ) {
    s = (*it);
    // remove accelerators
    s.replace( QRegExp( "&" ), "" ); // krazy:exclude=doublequote_chars
    list->addItem( s );
  }

  list->setCurrentRow( initialValue );
  list->setFocus();
  QObject::connect( list, SIGNAL(itemActivated(QListWidgetItem*)), dlg, SLOT(accept()) );
  restoreWindowSize("selectBox", dlg, QSize(247,174));

  int ret;
  if (dlg->exec())
    ret = list->currentRow();
  else
    ret = -1;

  saveWindowSize("selectBox", dlg->size());
  delete dlg;
  return ret;
}

// **** window geometry managing *********************************************

void KNHelper::saveWindowSize(const QString &name, const QSize &s)
{
  KConfigGroup c(knGlobals.config(), "WINDOW_SIZES");
  c.writeEntry(name, s);
}


void KNHelper::restoreWindowSize(const QString &name, QWidget *d, const QSize &defaultSize)
{
  KConfigGroup c(knGlobals.config(), "WINDOW_SIZES");

  QSize s=c.readEntry(name,defaultSize);

  if(s.isValid()) {
    QRect max = KGlobalSettings::desktopGeometry(QCursor::pos());
    if ( s.width() > max.width() ) s.setWidth( max.width()-5 );
    if ( s.height() > max.height() ) s.setHeight( max.height()-5 );
    d->resize(s);
  }
}

// **** scramble password strings **********************************************

const QString KNHelper::encryptStr(const QString& aStr)
{
  uint i,val,len = aStr.length();
  QString result;

  for (i=0; i<len; ++i)
  {
    val = aStr[i].toLatin1();
    val -= ' ';
    val = (255-' ') - val;
    result += (char)(val + ' ');
  }

  return result;
}


const QString KNHelper::decryptStr(const QString& aStr)
{
  return encryptStr(aStr);
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
  for(i=start;i<(int)text.length();++i)   // ok, the line is to long
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
      result+=(prefix+txt.left(breakPos)+'\n');
      txt.remove(0,breakPos+1);
    } else {
      result+=(prefix+txt+'\n');
      txt.clear();
    }
  }
}


QString KNHelper::rewrapStringList(const QStringList &text, int wrapAt, QChar quoteChar, bool stopAtSig, bool alwaysSpace)
{
  QString quoted, lastPrefix, thisPrefix, leftover, thisLine;
  int breakPos;

  for(QStringList::ConstIterator line=text.begin(); line!=text.end(); ++line) {

    if(stopAtSig && (*line)=="-- ")
      break;

    thisLine=(*line);
    if (!alwaysSpace && (thisLine[0]==quoteChar))
      thisLine.prepend(quoteChar);  // second quote level without space
    else
      thisLine.prepend( quoteChar + QString( ' ' ) );

    thisPrefix.clear();
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
    thisLine = thisLine.trimmed();

    if(!leftover.isEmpty()) {   // don't break paragraphs, tables and quote levels
      if(thisLine.isEmpty() || (thisPrefix!=lastPrefix) || thisLine.contains("  ") || thisLine.contains('\t'))
        appendTextWPrefix(quoted, leftover, wrapAt, lastPrefix);
      else
        thisLine.prepend( leftover + ' ' );
      leftover.clear();
    }

    if((int)(thisPrefix.length()+thisLine.length()) > wrapAt) {
      breakPos=findBreakPos(thisLine,wrapAt-thisPrefix.length());
      if(breakPos < (int)(thisLine.length())) {
        leftover=thisLine.right(thisLine.length()-breakPos-1);
        thisLine.truncate(breakPos);
      }
    }

    quoted+=thisPrefix+thisLine+'\n';
    lastPrefix=thisPrefix;
  }

  if (!leftover.isEmpty())
    appendTextWPrefix(quoted, leftover, wrapAt, lastPrefix);

  return quoted;
}

// **** misc. message-boxes **********************************************************

void KNHelper::displayInternalFileError(QWidget *w)
{
  KMessageBox::error((w!=0)? w : knGlobals.topWidget, i18n("Unable to load/save configuration.\nWrong permissions on home folder?\nYou should close KNode now to avoid data loss."));
}


void KNHelper::displayExternalFileError(QWidget *w)
{
  KMessageBox::error((w!=0)? w : knGlobals.topWidget, i18n("Unable to load/save file."));
}


void KNHelper::displayRemoteFileError(QWidget *w)
{
  KMessageBox::error((w!=0)? w : knGlobals.topWidget, i18n("Unable to save remote file."));
}


void KNHelper::displayTempFileError(QWidget *w)
{
  KMessageBox::error((w!=0)? w : knGlobals.topWidget, i18n("Unable to create temporary file."));
}



int KNHelper::findStringInFile( QFile * file, const char * str )
{
  QByteArray searchBuffer;
  int currentFilePos, pos;

  while ( !file->atEnd() ) {
    currentFilePos = file->pos();
    searchBuffer = file->read( 4096 );
    if ( searchBuffer.isEmpty() )
      return -1;

    pos = searchBuffer.indexOf( str );
    if ( pos < 0 ) {
      if ( !file->atEnd() )
        file->seek( file->pos() - strlen( str ) );
      else
        return -1;
    } else
      return currentFilePos + pos;
  }
  return -1;
}


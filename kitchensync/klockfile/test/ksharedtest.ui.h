/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include <kmessagebox.h>

void Form1::slotWriteUnlock()
{
  if( file->unlockWriteFile(ticket ) ){
    KMessageBox::error(this, "Unlocked worked");
  }else {
    KMessageBox::error(this, "Couldn't unlock. Did you lock before?" );
  }
}

void Form1::slotReadLock()
{
  ticket = file->requestReadTicket( );
  if(ticket==0){
  KMessageBox::error(this, "Couldn't read lock" );
  }else{
  KMessageBox::error(this, "Read Lock gained" );
  }
}

void Form1::slotReadUnlock()
{
  if( file->unlockReadFile(ticket) ) {
    KMessageBox::error(this, "Unlocked worked");
  }else {
    KMessageBox::error(this, "Couldn't unlock. Did you lock before?" );
  }
}

void Form1::slotWriteLock()
{
  ticket = file->requestWriteTicket( );
  if(ticket==0 ){
    KMessageBox::error(this, "Locked Writing? Didn't work");
  }else {
    KMessageBox::error(this, "Worked" );
  }
}

void Form1::init()
{
file = new KSharedFile("test.file" );
ticket=0l;
}

/*
* kfbrowsebtn.cpp -- Implementation of class KBrowseButton.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Tue Aug  4 03:44:44 EST 1998
*/

#include"kfbrowsebtn.h"

#include<assert.h>
#include<klocale.h>
#include<kfiledialog.h>

KBrowseButton::KBrowseButton(
  const QString & caption, 
	const QString & path,
  QWidget *parent,
  const char *name, 
	bool modal
)
	: QPushButton(caption.isEmpty() ? i18n( "Browse..." ) : caption, 
			parent, name ),
	_dialog( 0 ),
	_path(path),
	_modal ( modal )
{
	connect( this, SIGNAL(clicked()), this, SLOT(showDialog()) );
}

KBrowseButton::~KBrowseButton()
{
	delete _dialog;
}

QString KBrowseButton::path() const
{
	return _path;
}

void KBrowseButton::setPath(const QString & url)
{
	_path = url;

	if( _dialog ) {
		// change dialog path
		_dialog->setSelection( url );
	}
}

void KBrowseButton::showDialog()
{
	// create dialog if needed

	if ( !_dialog ) {
		_dialog = newBrowserDialog( _modal );
		assert( _dialog != 0 );
		connect( _dialog, SIGNAL(okClicked()),
			 this, SLOT(acceptPath()) );
	}
	//kdDebug() << "unixcfg: dialog setup" << endl;
	_dialog->show();
	//kdDebug() << "unixcfg: dialog shown, result = " << _dialog->result() << endl;
}

void KBrowseButton::setPathAndRaise(const QString& url)
{
  //kdDebug() << "unixcfg: " << url << " selected" << endl;
	_path = url;

	emit pathChanged( url );
}

void KBrowseButton::acceptPath()
{
  setPathAndRaise(_dialog->selectedFile());

}

KFileDialog *KFileBrowseButton::newBrowserDialog( bool modal )
{
	return new KFileDialog( path() , QString::null, 0, 0, modal );
}

KFileDialog *KDirBrowseButton::newBrowserDialog( bool modal )
{
  KFileDialog* fileDialog = new KFileDialog(path(), QString::null, 0, 0, modal );
  fileDialog->setMode( KFile::Directory );
  return fileDialog;
}
#include "kfbrowsebtn.moc"

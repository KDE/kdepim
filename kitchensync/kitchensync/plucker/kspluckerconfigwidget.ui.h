/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Holger Hans Peter Freyther <freyther@kde.org>

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
*/


/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/

void KSPluckerConfigWidget::init()
{
    urlJava->setMode(KFile::File|KFile::LocalOnly);
    urlPlucker->setMode(KFile::Directory|KFile::LocalOnly);
}

void KSPluckerConfigWidget::slotAdd()
{
    QString file = KFileDialog::getOpenFileName(QString::null,
					    "*.xml *.jxl|"+i18n("JPlucker Files"));

    /*
      * If the Text is Valid and we add it to the ListBox if
      * the text is not already present
      */
    if(!file.isEmpty() && !lstBoxFiles->findItem(file) )
	lstBoxFiles->insertItem(file, 0);
}


void KSPluckerConfigWidget::slotRemove()
{
    lstBoxFiles->removeItem( lstBoxFiles->currentItem() );
}


void KSPluckerConfigWidget::slotConfigOk()
{
    KSPlucker::PluckerConfig *conf = KSPlucker::PluckerConfig::self();

    conf->setJavaPath( urlJava->url() );
    conf->setPluckerPath( urlPlucker->url() );

    QStringList lst;
    for ( uint i = 0; i < lstBoxFiles->count(); ++i )
        lst.append(  lstBoxFiles->text( i ) );

    conf->setPluckerFiles( lst );
}

void KSPluckerConfigWidget::readConfig()
{
    KSPlucker::PluckerConfig *conf = KSPlucker::PluckerConfig::self();

    urlJava->setURL( conf->javaPath() );
    urlPlucker->setURL( conf->pluckerPath() );
    lstBoxFiles->insertStringList( conf->pluckerFiles() );

}

void KSPluckerConfigWidget::slotConfigureJXL()
{
    QString file = lstBoxFiles->currentText();
    if ( file.isEmpty() )
        return;

    KSPlucker::PluckerProcessHandler p( KSPlucker::PluckerProcessHandler::Configure, true, file);
    p.run();
}

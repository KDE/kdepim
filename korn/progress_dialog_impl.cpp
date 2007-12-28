/*
 * Copyright (C) 2004, Mart Kelder (mart.kde@hccnet.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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

#include "progress_dialog_impl.h"

DoubleProgressDialog::DoubleProgressDialog( QWidget *parent )
	: QDialog( parent ),
	Ui_DoubleProgressDialog()
{
	setupUi( this );
}

DoubleProgressDialog::~DoubleProgressDialog()
{
}

void DoubleProgressDialog::setText( const QString & str )
{
    lbText->setText( str );
}


void DoubleProgressDialog::setNumberOfBoxes( int number )
{
    pbBoxes->setRange( 0, number );
    pbBoxes->setValue( 0 );
}


void DoubleProgressDialog::setProgressOfBoxes( int number )
{
    pbBoxes->setValue( number );
}


void DoubleProgressDialog::setNumberOfSteps( int number )
{
    pbProgress->setRange( 0, number );
    pbProgress->setValue( 0 );
}


void DoubleProgressDialog::setProgress( int number )
{
    pbProgress->setValue( number );
}


void DoubleProgressDialog::cancelbutton()
{
    emit cancelPressed();
}

#include "progress_dialog_impl.moc"


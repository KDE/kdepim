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

#ifndef PROGRESS_DIALOG_IMPL_H
#define PROGRESS_DIALOG_IMPL_H

/**
 * @file
 *
 * This file contains the class DoubleProgressDialog.
 * That is just a progress dialog with two bars
 */

#include "progress_dialog.h"
#include <QDialog>

/**
 * This class is used for putting two progress bars into one dialog.
 * It is possible to add a text to the dialog
 */
class DoubleProgressDialog : public QDialog, public Ui_DoubleProgressDialog
{ Q_OBJECT
public:
	/**
	 * The constructor.
	 *
	 * @param parent the parent of the dialog
	 */
	DoubleProgressDialog( QWidget *parent );
	/**
	 * Virtual destructor
	 */
	virtual ~DoubleProgressDialog();

public slots:
	/**
	 * This function sets the text above the progress bars.
	 *
	 * @param str new text to put above the progress bars
	 */
	void setText( const QString& str );
	/**
	 * This function sets the total number of items for the major progress bar.
	 * In this program, the major progress bar is always used for boxes.
	 *
	 * @param number the total number of boxes
	 */
	void setNumberOfBoxes( int number );
	/**
	 * This function sets the progress of the major progress bar.
	 * In this program, the major progress bar is always used for boxes.
	 *
	 * @param number the progress of the major progress bar
	 */
	void setProgressOfBoxes( int number );
	/**
	 * This function sets the total number of items for the minor progress bar.
	 * 
	 * @param number the total number of steps in the minor progress bar
	 */
	void setNumberOfSteps( int number );
	/**
	 * This function sets the progress for the minor progress bar.
	 *
	 * @param number the progress of the minor progress bar
	 */
	void setProgress( int number );

private slots:
	/**
	 * This function is called when the user press the cancel button.
	 */
	void cancelbutton();

signals:
	/**
	 * This signal is emitted when the user press the cancel button.
	 * The pending operation should be cancelled.
	 */
	void cancelPressed();
};


#endif //PROGRESS_DIALOG_IMPL_H

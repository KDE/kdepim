/*
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "filteractionwithstring.h"

#include <QLineEdit>

#include <QTextDocument>

using namespace MailCommon;

FilterActionWithString::FilterActionWithString( const QString &name, const QString &label, QObject *parent )
    : FilterAction( name, label, parent )
{
}

bool FilterActionWithString::isEmpty() const
{
    return mParameter.trimmed().isEmpty();
}

QWidget* FilterActionWithString::createParamWidget( QWidget *parent ) const
{
    QLineEdit *lineEdit = new QLineEdit( parent );
    lineEdit->setClearButtonEnabled( true );
    //QT5 lineEdit->setTrapReturnKey(true);
    lineEdit->setText( mParameter );

    connect( lineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(filterActionModified()) );

    return lineEdit;
}

void FilterActionWithString::applyParamWidgetValue( QWidget *paramWidget )
{
    mParameter = static_cast<QLineEdit*>( paramWidget )->text();
}

void FilterActionWithString::setParamWidgetValue( QWidget *paramWidget ) const
{
    static_cast<QLineEdit*>( paramWidget )->setText( mParameter );
}

void FilterActionWithString::clearParamWidget( QWidget *paramWidget ) const
{
    static_cast<QLineEdit*>( paramWidget )->clear();
}

void FilterActionWithString::argsFromString( const QString &argsStr )
{
    mParameter = argsStr;
}

QString FilterActionWithString::argsAsString() const
{
    return mParameter;
}

QString FilterActionWithString::displayString() const
{
    return label() + QLatin1String( " \"" ) + Qt::escape( argsAsString() ) + QLatin1String( "\"" );
}



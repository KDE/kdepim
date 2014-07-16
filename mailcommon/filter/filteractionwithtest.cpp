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

#include "filteractionwithtest.h"

#include "soundtestwidget.h"

#include <QTextDocument>

using namespace MailCommon;

FilterActionWithTest::FilterActionWithTest( const QString &name, const QString &label, QObject *parent )
    : FilterAction( name, label, parent )
{
}

FilterActionWithTest::~FilterActionWithTest()
{
}

bool FilterActionWithTest::isEmpty() const
{
    return mParameter.trimmed().isEmpty();
}

QWidget* FilterActionWithTest::createParamWidget( QWidget *parent ) const
{
    SoundTestWidget *soundWidget = new SoundTestWidget( parent );
    soundWidget->setUrl( mParameter );

    connect( soundWidget, SIGNAL(textChanged(QString)),
             this, SIGNAL(filterActionModified()) );

    return soundWidget;
}

void FilterActionWithTest::applyParamWidgetValue( QWidget *paramWidget )
{
    mParameter = static_cast<SoundTestWidget*>( paramWidget )->url();
}

void FilterActionWithTest::setParamWidgetValue( QWidget *paramWidget ) const
{
    static_cast<SoundTestWidget*>( paramWidget )->setUrl( mParameter );
}

void FilterActionWithTest::clearParamWidget( QWidget *paramWidget ) const
{
    static_cast<SoundTestWidget*>( paramWidget )->clear();
}

void FilterActionWithTest::argsFromString( const QString &argsStr )
{
    mParameter = argsStr;
}

QString FilterActionWithTest::argsAsString() const
{
    return mParameter;
}

QString FilterActionWithTest::displayString() const
{
    return label() + QLatin1String( " \"" ) + Qt::escape( argsAsString() ) + QLatin1String( "\"" );
}


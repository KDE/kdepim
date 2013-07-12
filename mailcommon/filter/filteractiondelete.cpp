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

#include "filteractiondelete.h"

#include <KDE/KLocale>
#include <KDE/KColorScheme>

#include <QLabel>

using namespace MailCommon;

FilterActionDelete::FilterActionDelete( QObject *parent )
  : FilterActionWithNone( QLatin1String("delete"), i18n( "Delete Message" ), parent )
{
}

FilterAction::ReturnCode FilterActionDelete::process( ItemContext &context ) const
{
  context.setDeleteItem();
  return GoOn;
}

SearchRule::RequiredPart FilterActionDelete::requiredPart() const
{
  return SearchRule::Envelope;
}


QWidget* FilterActionDelete::createParamWidget( QWidget *parent ) const
{
    QLabel *lab = new QLabel(parent);
    QPalette pal = lab->palette();
    KColorScheme scheme(QPalette::Active, KColorScheme::View);
    pal.setColor(QPalette::WindowText, scheme.foreground(KColorScheme::NegativeText).color());
    lab->setPalette(pal);
    lab->setText(i18n("Be careful, mails will be removed."));
    return lab;
}

FilterAction* FilterActionDelete::newAction()
{
  return new FilterActionDelete;
}

bool FilterActionDelete::canConvertToSieve() const
{
    return true;
}

QString FilterActionDelete::sieveCode() const
{
    return QLatin1String("discard;");
}


#include "filteractiondelete.moc"

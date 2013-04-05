/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "sieveactionredirect.h"

#include <KLocale>
#include <KLineEdit>

using namespace KSieveUi;

SieveActionRedirect::SieveActionRedirect(QObject *parent)
    : SieveAction(QLatin1String("redirect"), i18n("Redirect"), parent)
{
}

SieveAction *SieveActionRedirect::newAction()
{
    return new SieveActionRedirect;
}

QWidget *SieveActionRedirect::createParamWidget( QWidget *parent ) const
{
    KLineEdit *edit = new KLineEdit(parent);
    edit->setObjectName("RedirectEdit");
    return edit;
}

QString SieveActionRedirect::code(QWidget *w) const
{
    const KLineEdit *edit = w->findChild<KLineEdit*>( "RedirectEdit" );
    const QString text = edit->text();
    return QString::fromLatin1("  redirect \"%1\";").arg(text);
}

#include "sieveactionredirect.moc"

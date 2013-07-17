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

#include "sieveactionconvert.h"
#include "autocreatescripts/commonwidgets/selectconvertparameterwidget.h"
#include "autocreatescripts/commonwidgets/selectmimetypecombobox.h"

#include <KLocale>
#include <KLineEdit>

#include <QLabel>
#include <QHBoxLayout>

using namespace KSieveUi;
SieveActionConvert::SieveActionConvert(QObject *parent)
    : SieveAction(QLatin1String("convert"), i18n("Convert"), parent)
{
}

SieveAction* SieveActionConvert::newAction()
{
    return new SieveActionConvert;
}

QWidget *SieveActionConvert::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    QLabel *lab = new QLabel(i18n("From:"));
    lay->addWidget(lab);

    SelectMimeTypeComboBox *fromMimeType = new SelectMimeTypeComboBox;
    fromMimeType->setObjectName(QLatin1String("from"));
    lay->addWidget(fromMimeType);

    lab = new QLabel(i18n("To:"));
    lay->addWidget(lab);

    SelectMimeTypeComboBox *toMimeType = new SelectMimeTypeComboBox;
    toMimeType->setObjectName(QLatin1String("to"));
    lay->addWidget(toMimeType);

    lab = new QLabel(i18n("Parameters:"));
    lay->addWidget(lab);

    SelectConvertParameterWidget *params = new SelectConvertParameterWidget;
    params->setObjectName(QLatin1String("params"));
    lay->addWidget(params);

    return w;
}

void SieveActionConvert::setParamWidgetValue(QWidget *w ) const
{
    const SelectMimeTypeComboBox *fromMimeType = w->findChild<SelectMimeTypeComboBox*>( QLatin1String("from") );
    //fromMimeType->setCode();
    const SelectMimeTypeComboBox *toMimeType = w->findChild<SelectMimeTypeComboBox*>( QLatin1String("to") );
    //toMimeType->setCode();
    const SelectConvertParameterWidget *params = w->findChild<SelectConvertParameterWidget*>( QLatin1String("params") );
    //params->setCode();
}

QString SieveActionConvert::code(QWidget *w) const
{
    QString result = QLatin1String("convert ");
    const SelectMimeTypeComboBox *fromMimeType = w->findChild<SelectMimeTypeComboBox*>( QLatin1String("from") );
    const QString fromMimeTypeStr = fromMimeType->code();
    result += QString::fromLatin1("%1 ").arg(fromMimeTypeStr);

    const SelectMimeTypeComboBox *toMimeType = w->findChild<SelectMimeTypeComboBox*>( QLatin1String("to") );
    const QString toMimeTypeStr = toMimeType->code();
    result += QString::fromLatin1("%1 ").arg(toMimeTypeStr);

    const SelectConvertParameterWidget *params = w->findChild<SelectConvertParameterWidget*>( QLatin1String("params") );
    const QString paramsStr = params->code();
    if (!paramsStr.isEmpty()) {
        result += paramsStr;
    }
    result += QLatin1Char(';');
    return result;
}


QStringList SieveActionConvert::needRequires(QWidget *parent) const
{
    return QStringList() << QLatin1String("convert");
}

bool SieveActionConvert::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveActionConvert::serverNeedsCapability() const
{
    return QLatin1String("convert");
}

QString SieveActionConvert::help() const
{
    return i18n("The \"convert\" action specifies that all body parts with a media type equal to \"media-type\" be converted to the media type in \"media-type\" using conversion parameters.");
}

#include "sieveactionconvert.moc"

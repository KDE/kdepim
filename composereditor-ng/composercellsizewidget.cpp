/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/
#include "composercellsizewidget.h"

#include <KComboBox>
#include <KLocale>
#include <KDebug>

#include <QSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QGridLayout>

namespace ComposerEditorNG
{

class ComposerCellSizeWidgetPrivate
{
public:
    enum TypeSize {
        Percentage,
        Fixed
    };
    ComposerCellSizeWidgetPrivate(ComposerCellSizeWidget *qq)
        :q(qq)
    {
        QHBoxLayout *layout = new QHBoxLayout;
        check = new QCheckBox;
        layout->addWidget(check);

        size = new QSpinBox;
        size->setMinimum( 1 );
        size->setMaximum( 100 );
        layout->addWidget(size);

        typeSize = new KComboBox;
        q->connect( typeSize, SIGNAL(activated(int)),q,SLOT(slotTypeChanged(int)) );
        // xgettext: no-c-format
        typeSize->addItem( i18n( "% of windows" ), Percentage );
        typeSize->addItem( i18n( "pixels" ), Fixed );
        layout->addWidget(typeSize);
        q->setLayout(layout);

    }

    void slotTypeOfLengthChanged(int index);
    void setValue(const QString&);
    void setType(TypeSize type);
    QString value() const;

    QSpinBox *size;
    KComboBox *typeSize;
    QCheckBox *check;
    ComposerCellSizeWidget *q;
};

void ComposerCellSizeWidgetPrivate::setValue(const QString& val)
{
    if(val.isEmpty()) {
        check->setChecked(false);
    } else {
        check->setChecked(true);
        QString valStr(val);
        if(valStr.endsWith(QLatin1Char('%'))) {
            setType(Percentage);
            valStr.chop(1);
            size->setValue(valStr.toInt());
        } else {
            setType(Fixed);
            size->setValue(valStr.toInt());
        }
    }
}

void ComposerCellSizeWidgetPrivate::setType(TypeSize type)
{
  const int index = typeSize->findData(QVariant(type));
  typeSize->setCurrentIndex(index);
  slotTypeOfLengthChanged(index);
}


void ComposerCellSizeWidgetPrivate::slotTypeOfLengthChanged(int index)
{
    switch ( index ) {
    case 0:
        size->setMaximum( 100 );
        size->setValue( qMin( size->value(), 100 ) );
        break;
    case 1:
        size->setMaximum( 9999 );
        break;
    default:
        kDebug() << " index not defined " << index;
        break;
    }
}

QString ComposerCellSizeWidgetPrivate::value() const
{
    if(check->isChecked()) {
        if((TypeSize)typeSize->itemData( typeSize->currentIndex() ).toInt() == Percentage) {
            return QString::fromLatin1("%1%").arg(size->value());
        }
        return QString::number(size->value());
    } else {
        return QString();
    }
}

ComposerCellSizeWidget::ComposerCellSizeWidget(QWidget *parent)
    :QWidget(parent), d(new ComposerCellSizeWidgetPrivate(this))
{
}

ComposerCellSizeWidget::~ComposerCellSizeWidget()
{
    delete d;
}

void ComposerCellSizeWidget::setValue(const QString& val)
{
    d->setValue(val);
}

QString ComposerCellSizeWidget::value() const
{
    return d->value();
}

}

#include "composercellsizewidget.moc"

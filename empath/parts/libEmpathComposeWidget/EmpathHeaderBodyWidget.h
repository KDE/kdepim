/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef EMPATH_HEADER_BODY_WIDGET_H
#define EMPATH_HEADER_BODY_WIDGET_H

// Qt includes
#include <qwidget.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qmessagebox.h>

// KDE includes
#include <klineedit.h>
#include <klocale.h>
#include <kiconloader.h>

/**
 * Abstract base class for widgets in which the user can edit a header,
 * such as EmpathAddressSelectionWidget for addresses and EmpathText-
 * SelectionWidget for the subject. Both use a KLineEdit widget for this
 * but if you are crazy enough you can use a combobox for example.
 */
class EmpathHeaderBodyWidget : public QWidget
{
    Q_OBJECT
    
    public:
    
        EmpathHeaderBodyWidget(QWidget * parent = 0);
        virtual ~EmpathHeaderBodyWidget();

        virtual QString text() const = 0;
        virtual void setText(const QString &) = 0;
};

class EmpathTextHeaderBodyWidget : public EmpathHeaderBodyWidget
{
    Q_OBJECT
    
    public:
    
        EmpathTextHeaderBodyWidget(QWidget * parent = 0)
            :   EmpathHeaderBodyWidget(parent)
        {
            // Empty.
        }

        virtual ~EmpathTextHeaderBodyWidget()
        {
            // Empty.
        }

        QString text() const
        {
            return le_address_->text();
        }

        void setText(const QString & s)
        {
            le_address_->setText(s);
        }

    private:

        KLineEdit   * le_address_;
};

class EmpathAddressHeaderBodyWidget : public EmpathHeaderBodyWidget
{
    Q_OBJECT
    
    public:
    
        EmpathAddressHeaderBodyWidget(QWidget * parent = 0)
            :   EmpathHeaderBodyWidget(parent)
        {
            le_address_ = new KLineEdit(this, "le_address");

            QPushButton * pb_browse_ = new QPushButton(this, "pb_browse");

            pb_browse_->setPixmap(BarIcon("pointingHand")), 
            pb_browse_->setFixedWidth(pb_browse_->sizeHint().width());

            QHBoxLayout * layout = new QHBoxLayout(this);

            layout->addWidget(le_address_);
            layout->addWidget(pb_browse_);

            QObject::connect(
                pb_browse_, SIGNAL(clicked()),
                this,       SLOT(s_browseClicked())
            );

            setFocusProxy(le_address_);
        }

        virtual ~EmpathAddressHeaderBodyWidget()
        {
            // Empty.
        }

        QString text() const
        {
            return le_address_->text();
        }

        void setText(const QString & s)
        {
            le_address_->setText(s);
        }

        KLineEdit * lineEdit() { return le_address_; };
        
    protected slots:

        void s_browseClicked()
        {
            QMessageBox::information(
                this,
                "Empath",
                i18n("Sorry, the addressbook isn't ready for use yet."),
                i18n("OK")
            );
        }

    private:

        KLineEdit   * le_address_;
};



#endif

// vim:ts=4:sw=4:tw=78

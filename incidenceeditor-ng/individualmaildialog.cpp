/*
  Copyright (c) 2014 Sandro Knauß <knauss@kolabsys.com>

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

#include "individualmaildialog.h"

#include <KLocalizedString>
#include <KDebug>

#include <QGridLayout>
#include <QComboBox>
#include <QLabel>

IncidenceEditorNG::IndividualMailDialog::IndividualMailDialog(QWidget *parent,
                                                              const QString &question, const QStringList &attendees,
                                                              const KGuiItem &buttonYes, const KGuiItem &buttonNo)
    : KDialog(parent)
{
    setCaption( i18n("Group Scheduling Email") );
    setButtons( KDialog::Yes | KDialog::No | KDialog::Details);
    setButtonText(KDialog::Details, i18n("Individual mailsettings"));
    setButtonGuiItem(KDialog::Yes, buttonYes);
    setButtonGuiItem(KDialog::No, buttonNo);

    QWidget *widget = new QWidget();
    QGridLayout *layout = new QGridLayout();
    int row = 0;
    foreach(const QString &attendee, attendees) {
        QComboBox *options = new QComboBox();
        options->addItem(i18n("send update"),QVariant(attendee));
        options->addItem(i18n("send no update"),QVariant(attendee));
        options->addItem(i18n("edit mail"),QVariant(attendee));
        mAttendeeDescition[attendee] = options;

        layout->addWidget(new QLabel(attendee),row,0);
        layout->addWidget(options,row,1);
        row++;
    }
    widget->setLayout(layout);
    widget->sizePolicy().setHorizontalStretch(1);
    widget->sizePolicy().setVerticalStretch(1);

    QWidget *mW = new QLabel(question);

    setMainWidget(mW);
    setDetailsWidget(widget);
}


IncidenceEditorNG::IndividualMailDialog::~IndividualMailDialog()
{

}

const QStringList &IncidenceEditorNG::IndividualMailDialog::editAttendees()
{
    QStringList *edit = new QStringList();
    //TODO: delete list?
    QStringList attendees = mAttendeeDescition.keys();
    foreach(const QString &attendee, attendees) {
        if (mAttendeeDescition[attendee]->currentText() == i18n("edit mail")) {
            edit->append(attendee);
        }
    }
    return *edit;
}

const QStringList &IncidenceEditorNG::IndividualMailDialog::updateAttendees()
{
    QStringList *update = new QStringList();
    //TODO: delete list?
    QStringList attendees = mAttendeeDescition.keys();
    foreach(const QString &attendee, attendees) {
        if (mAttendeeDescition[attendee]->currentText() == i18n("send update")) {
            update->append(attendee);
        }
    }
    return *update;
}
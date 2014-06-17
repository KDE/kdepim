/*
 * Copyright 2014  Sandro Knauß <knauss@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef INCIDENCEEDITOR_ATTENDEECOMBOBOXDELEGATE_H
#define INCIDENCEEDITOR_ATTENDEECOMBOBOXDELEGATE_H

#include <kglobalsettings.h>

#include <QStyledItemDelegate>
#include <QModelIndex>
#include <QIcon>
#include <QString>


namespace IncidenceEditorNG
{

  /**
   * class to show a Icon and Text for an Attendee
   * you have to set the Items via addItem to have a list to choose from.
   * saves the option as int in the model
   */
class AttendeeComboBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    AttendeeComboBoxDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    bool eventFilter ( QObject * editor, QEvent * event );

    void addItem(const QIcon&, const QString&);
    void clear();

    void setToolTip(const QString&);
    void setWhatsThis(const QString&);

    /** choose this index, if the item in the model is unknown
     */
    void setStandardIndex(int);

public slots:
    bool helpEvent( QHelpEvent* event, QAbstractItemView* view, const QStyleOptionViewItem& option, const QModelIndex& index );

private slots:
  void doCloseEditor(QWidget *editor);
  void rightPressed();
  void leftPressed();

private:
    /** all entries to choose from */
    QList<QPair<QIcon, QString> > entries;
    /**fallback index */
    int standardIndex;
    QString toolTip;
    QString whatsThis;
};

/** show a AttendeeLineEdit as editor */
class AttendeeLineEditDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    AttendeeLineEditDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void setCompletionMode( KGlobalSettings::Completion mode);

public slots:
    bool helpEvent( QHelpEvent* event, QAbstractItemView* view, const QStyleOptionViewItem& option, const QModelIndex& index );

private slots:
    void rightPressed();
    void leftPressed();

private:
    QString toolTip;
    QString whatsThis;
    KGlobalSettings::Completion completionMode;
};

}

#endif
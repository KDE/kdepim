/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/
#ifndef KAMMU_STATUSBAR_H
#define KAMMU_STATUSBAR_H

#include <q3hbox.h>
#include <q3vbox.h>
#include <q3scrollview.h>
//Added by qt3to4:
#include <QLabel>
#include <libkmobiletools/engine.h>
#include <q3ptrlist.h>
#include <qprogressbar.h>

class KPushButton;
class QLabel;
class KStatusBar;
class StatusBarJob;
class QLabel;
namespace KMobileTools
{
    class OverlayWidget;
}

class StatusBarScrollView : public Q3ScrollView {
    Q_OBJECT
    public:
        explicit StatusBarScrollView( QWidget * parent=0, const char * name=0, Qt::WFlags f=0 );
        ~StatusBarScrollView();
        QSize sizeHint() const;
        QSize minimumSizeHint() const;
        Q3VBox *getVBox() { return vbox; }
    protected:
        virtual void resizeContents ( int w, int h );
        Q3VBox *vbox;
};


class StatusBarProgressBox : public Q3HBox {
    Q_OBJECT
public:
    explicit StatusBarProgressBox( KStatusBar *statusbar, QWidget * parent = 0, const char * name = 0 );
    ~StatusBarProgressBox();
    Q3VBox *statusItemsBox() { return itemsBox; }
    private:
        KPushButton *showHideButton;
        QProgressBar *generalProgress;
        bool b_shown;
        QWidget *parentWidget;
        KMobileTools::OverlayWidget *overlay;
        Q3VBox *itemsBox;
        StatusBarScrollView *scrollView;
        int jobsCount;
        Q3PtrList<StatusBarJob> jobs;

    public slots:
        void slotShowHide();
        void slotJobEnqueued(KMobileTools::Job *job);
        void slotDeletedJob(StatusBarJob* job);
        void countTotalProgress();
    signals:
        void totalProgressChanged(int);
};

class SingleJobProgressBox : public Q3HBox {
    Q_OBJECT
    public:
        SingleJobProgressBox( int jobType,  const QString &description, QWidget * parent = 0, const char * name = 0 );
        ~SingleJobProgressBox();
        int progress() { return itemProgress->value(); }
        const QString &iconLabelName() { return s_itemLabelName; }
    private:
        QString s_itemLabelName;
        QProgressBar *itemProgress;
        QLabel *itemNameLabel;
    public slots:
        void setPercent(int p);
};

class StatusBarJob : public QObject {
    Q_OBJECT
    public:
        StatusBarJob( const QString &description, KMobileTools::Job *job, StatusBarProgressBox * parent, const char * name = 0 );
        ~StatusBarJob();
        int progress() { return box->progress(); }
    public slots:
        void jobDone();
        void deleteThis();
        void hide() { box->hide(); }
        void show() { box->show(); }
    private:
        SingleJobProgressBox *box;
        StatusBarProgressBox *parentBox;
        QLabel *q_iconLabel;
};

#endif

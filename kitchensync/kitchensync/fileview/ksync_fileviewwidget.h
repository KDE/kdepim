#ifndef KSYNC_FILEVIEWWIDGET_H
#define KSYNC_FILEVIEWWIDGET_H

#include <qwidget.h>
#include <qsplitter.h>

#include <kparts/mainwindow.h>
#include <kparts/partmanager.h>
#include <manipulatorpart.h>


namespace KitchenSync{

    class KSyncFileviewWidget : public KParts::MainWindow {
        // : public QWidget     {
    Q_OBJECT

    public:
        KSyncFileviewWidget( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
        ~KSyncFileviewWidget();

        void openURLHost( const KURL & url);
        void openURLClient( const KURL & url);

    private:
        KParts::PartManager *m_manager;
        KParts::ReadOnlyPart *m_part1;
        KParts::ReadOnlyPart *m_part2;
        QSplitter *m_splitter;
    };
};

#endif

#ifdef __GNUG__
# pragma interface "EmpathComposeWidget.h"
#endif

#ifndef EMPATHCOMPOSEWIDGET_H
#define EMPATHCOMPOSEWIDGET_H

// Qt includes
#include <qwidget.h>
#include <qsplitter.h>
#include <qlistview.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcombo.h>
#include <qdict.h>
#include <qdatetime.h>
#include <qfileinfo.h>
#include <qmultilinedit.h>

// KDE includes
#include <keditcl.h>

// Local includes
#include "Empath.h"
#include "EmpathDefines.h"
#include "EmpathURL.h"
#include "EmpathHeaderSpecWidget.h"
#include <RMM_Message.h>

class EmpathAttachmentListWidget;

/**
 * The container for the various widgets used when composing.
 */
class EmpathComposeWidget : public QWidget
{
    Q_OBJECT

    public:
        
        /**
         * Standard ctor
         */
        EmpathComposeWidget(QWidget * parent = 0, const char * name = 0);

        EmpathComposeWidget(
            Empath::ComposeType t, const EmpathURL &,
            QWidget * parent = 0, const char * name = 0);
    
        EmpathComposeWidget(
            const QString &,
            QWidget * parent = 0, const char * name = 0);
        
        /**
         * dtor
         */
        ~EmpathComposeWidget();

        /**
         * The message we're editing.
         */
        RMM::RMessage message();

        /**
         * Test if there are any attachments for this message.
         */
        bool messageHasAttachments();
        
        void init() { _init(); }
        
        void bugReport();
        
        bool haveTo();
        bool haveSubject();
        
    protected slots:
        
        void    s_editorDone(bool ok, QCString text);
    
        void    s_cut();
        void    s_copy();
        void    s_paste();
        void    s_selectAll();
        
        void    s_addAttachment();
        void    s_editAttachment();
        void    s_removeAttachment();
        
    private:

        void    _init();
        
        void    _reply(bool toAll = false);
        void    _forward();
        
        void    _spawnExternalEditor(const QCString & text);
        
        void    _addExtraHeaders();
        
        void    _addHeader(const QString &, const QString & = QString::null);
        
        void    _addInvisibleHeader
            (const QString &, const QString & = QString::null);
        
        void    _set(const QString &, const QString &);
        QString    _get(const QString &);
        
        void    _lineUpHeaders();
        
        QCString _envelope();
        QCString _body();
        
        QCString _referenceHeaders();
        QCString _visibleHeaders();
        QCString _invisibleHeaders();
        QCString _stdHeaders();
        
        EmpathAttachmentListWidget * attachmentWidget_;

        QMultiLineEdit   * editorWidget_;
        QListView        * lv_attachments_;
        QVBoxLayout      * headerLayout_;
        QLabel           * l_subject_;
        QLineEdit        * le_subject_;
        
        QList<EmpathHeaderSpecWidget> headerSpecList_;
        RMM::RHeaderList invisibleHeaders_;

        Empath::ComposeType    composeType_;

        EmpathURL    url_;
        QString        recipient_;
        
        int            maxSizeColOne_;
};

#endif
// vim:ts=4:sw=4:tw=78

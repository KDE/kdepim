noinst_LTLIBRARIES = libkdgantt.la

libkdgantt_la_SOURCES = \
			itemAttributeDialog.ui \
			KDGanttView.cpp			\
			KDGanttViewEventItem.cpp	\
			KDGanttViewItem.cpp		\
			KDGanttViewSubwidgets.cpp	\
			KDGanttViewSummaryItem.cpp	\
			KDGanttViewTaskItem.cpp		\
			KDGanttViewTaskLink.cpp		\
			KDGanttViewTaskLinkGroup.cpp	\
			KDMinimizeSplitter.cpp		\
			KDSemiSizingControl.cpp		\
			KDSizingControl.cpp		\
			KDXMLTools.cpp			

noinst_HEADERS =	\
			KDGanttViewSubwidgets.h		\
			KDMinimizeSplitter.h		\
			KDSemiSizingControl.h		\
			KDSizingControl.h		\
			KDGanttViewEventItem.h		\
			KDGanttView.h			\
			KDGanttViewItem.h		\
			KDGanttViewSummaryItem.h	\
			KDGanttViewTaskItem.h		\
			KDGanttViewTaskLinkGroup.h	\
			KDGanttViewTaskLink.h		\
			KDXMLTools.h			\
			itemAttributeDialog.ui.h	\
			qlayoutengine_p.h

AM_CPPFLAGS = $(all_includes)
libkdgantt_la_METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/kdgantt.pot

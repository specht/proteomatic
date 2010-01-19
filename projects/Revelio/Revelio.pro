TARGET = Revelio

include(../base.pro)

win32 {
    RC_FILE = Revelio.rc
}

# Input
HEADERS += \
	../../src/RevelioMainWindow.h \
	../../src/Surface.h \
	../../src/FileTrackerNode.h \
    ../../src/Desktop.h \
    ../../src/DesktopBox.h \
    ../../src/DesktopBoxFactory.h \
    ../../src/IDesktopBox.h \
    ../../src/IFileBox.h \
    ../../src/InputGroupProxyBox.h \
    ../../src/IScriptBox.h \
    ../../src/FileListBox.h \
    ../../src/OutFileListBox.h \
    ../../src/PipelineMainWindow.h \
    ../../src/ScriptBox.h \
	
SOURCES += \
	../../src/RevelioMainWindow.cpp \
	../../src/RevelioMain.cpp \
	../../src/Surface.cpp \
	../../src/FileTrackerNode.cpp \
    ../../src/Desktop.cpp \
    ../../src/DesktopBox.cpp \
    ../../src/DesktopBoxFactory.cpp \
    ../../src/FileListBox.cpp \
    ../../src/InputGroupProxyBox.cpp \
    ../../src/OutFileListBox.cpp \
    ../../src/PipelineMainWindow.cpp \
    ../../src/ScriptBox.cpp \
	
RESOURCES += \
	../../src/Proteomatic.qrc \
	
QT += sql

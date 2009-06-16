TARGET = bin/ProteomaticPipeline

include(../base.pro)

win32 {
    RC_FILE = ProteomaticPipeline.rc
}

# Input
HEADERS += \
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
    ../../src/Desktop.cpp \
    ../../src/DesktopBox.cpp \
    ../../src/DesktopBoxFactory.cpp \
	../../src/FileListBox.cpp \
	../../src/InputGroupProxyBox.cpp \
	../../src/OutFileListBox.cpp \
	../../src/PipelineMainWindow.cpp \
	../../src/ProteomaticPipelineMain.cpp \
	../../src/ScriptBox.cpp \
	
RESOURCES += \
	../../src/Proteomatic.qrc \

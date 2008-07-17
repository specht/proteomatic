TEMPLATE = app
win32 {
    #TEMPLATE = vcapp
    RC_FILE = Proteomatic.rc
}

OBJECTS_DIR = ../../obj/
MOC_DIR = ../../obj/
RCC_DIR = ../../obj/

TARGET = ProteomaticPipeline
DESTDIR = ../../../

QT += gui network

CONFIG += debug_and_release

# Input
HEADERS += \
	../../src/CiListWidgetItem.h \
	../../src/ClickableLabel.h \
	../../src/ConsoleString.h \
    ../../src/Desktop.h \
    ../../src/DesktopBox.h \
	../../src/FileList.h \
	../../src/FoldedHeader.h \
	../../src/LocalScript.h \
	../../src/PipelineMainWindow.h \
	../../src/Proteomatic.h \
	../../src/RefPtr.h \
	../../src/RemoteScript.h \
	../../src/RubyWindow.h \
	../../src/Script.h \
	../../src/ScriptFactory.h \
    ../../src/StopWatch.h \

SOURCES += \
	../../src/ClickableLabel.cpp \
	../../src/ConsoleString.cpp \
    ../../src/Desktop.cpp \
    ../../src/DesktopBox.cpp \
	../../src/FileList.cpp \
	../../src/FoldedHeader.cpp \
	../../src/LocalScript.cpp \
	../../src/PipelineMainWindow.cpp \
	../../src/Proteomatic.cpp \
	../../src/ProteomaticPipelineMain.cpp \
	../../src/RemoteScript.cpp \
	../../src/RubyWindow.cpp \
	../../src/Script.cpp \
	../../src/ScriptFactory.cpp \
    ../../src/StopWatch.cpp \
	
RESOURCES += \
	../../src/Proteomatic.qrc \

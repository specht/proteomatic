TEMPLATE = app
win32 {
    RC_FILE = Proteomatic.rc
    CONFIG += embed_manifest_exe
}

macx {
	CONFIG -= app_bundle
	CONFIG += ppc
	ICON = ../../src/icons/proteomatic.icns
}

OBJECTS_DIR = ../../obj/
MOC_DIR = ../../obj/
RCC_DIR = ../../obj/

TARGET = Proteomatic
DESTDIR = ../../../

QT += gui network

CONFIG += debug_and_release

# Input
HEADERS += \
	../../src/CiListWidgetItem.h \
	../../src/ClickableLabel.h \
	../../src/ConsoleString.h \
	../../src/FileList.h \
	../../src/FoldedHeader.h \
	../../src/LocalScript.h \
	../../src/ProfileManager.h \
	../../src/Proteomatic.h \
	../../src/RefPtr.h \
	../../src/RemoteScript.h \
	../../src/RubyWindow.h \
	../../src/Script.h \
	../../src/ScriptHelper.h \
	../../src/ScriptFactory.h \
	../../src/SizeWatchWidget.h \
	../../src/TicketWindow.h \
	
SOURCES += \
	../../src/ClickableLabel.cpp \
	../../src/ConsoleString.cpp \
	../../src/FileList.cpp \
	../../src/FoldedHeader.cpp \
	../../src/LocalScript.cpp \
	../../src/ProfileManager.cpp \
	../../src/Proteomatic.cpp \
	../../src/ProteomaticMain.cpp \
	../../src/RemoteScript.cpp \
	../../src/RubyWindow.cpp \
	../../src/Script.cpp \
	../../src/ScriptHelper.cpp \
	../../src/ScriptFactory.cpp \
	../../src/TicketWindow.cpp \
	
RESOURCES += \
	../../src/Proteomatic.qrc \

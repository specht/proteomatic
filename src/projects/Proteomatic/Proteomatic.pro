TEMPLATE = app

win32 {
    RC_FILE = Proteomatic.rc
}

CONFIG += debug_and_release

macx {
	CONFIG -= app_bundle
	CONFIG += ppc
	ICON = ../../src/icons/proteomatic.icns
}

TARGET = Proteomatic

CONFIG(debug, debug|release) {
	OBJECTS_DIR = ../../obj/debug/
	MOC_DIR = ../../obj/debug/
	RCC_DIR = ../../obj/debug/
	TARGET = $$join(TARGET,,,_debug)
}
else {
	OBJECTS_DIR = ../../obj/release/
	MOC_DIR = ../../obj/release/
	RCC_DIR = ../../obj/release/
}

DESTDIR = ../../../

QT += gui network

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

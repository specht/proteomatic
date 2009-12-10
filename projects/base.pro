TEMPLATE = app

win32 {
	INCLUDEPATH += c:/Qt/2009.04/mingw/include
}

CONFIG += debug_and_release

macx {
	CONFIG += app_bundle
	CONFIG += x86 ppc
	QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.4
	ICON = ../../src/icons/proteomatic-pipeline.icns
	INCLUDEPATH += '/Users/michael/programming/ext/include'
	INCLUDEPATH += '/Users/dimitris/michael/ext/include'
}

CONFIG(debug, debug|release) {
	OBJECTS_DIR = ../../obj/debug/
	MOC_DIR = ../../obj/debug/
	RCC_DIR = ../../obj/debug/
	TARGET = $$join(TARGET,,,_debug)
	DEFINES += DEBUG
}
else {
	OBJECTS_DIR = ../../obj/release/
	MOC_DIR = ../../obj/release/
	RCC_DIR = ../../obj/release/
}

DESTDIR = ../../

QT = core gui network

INCLUDEPATH += ../../src/ ../../src/dialogs

MYLIBPATH = /usr/local/lib

win32 {
	MYLIBPATH = c:\Qt\2009.04\mingw\lib\
}
macx {
	MYLIBPATH = /Users/dimitris/michael/ext/lib
}

LIBS += $$MYLIBPATH/libyaml-cpp.a $$MYLIBPATH/libmd5.a


# Input files
HEADERS += \
	../../src/CiListWidgetItem.h \
	../../src/ClickableLabel.h \
	../../src/ConsoleString.h \
	../../src/FileList.h \
	../../src/FoldedHeader.h \
	../../src/HintLineEdit.h \
	../../src/IScript.h \
	../../src/LocalScript.h \
    ../../src/NoSlashValidator.h \
	../../src/ProfileManager.h \
	../../src/Proteomatic.h \
	../../src/RefPtr.h \
	../../src/RemoteScript.h \
	../../src/RubyWindow.h \
	../../src/Script.h \
	../../src/ScriptFactory.h \
    ../../src/StopWatch.h \
	../../src/Tango.h \
	../../src/TicketWindow.h \
	../../src/UnclickableLabel.h \
	../../src/version.h \
	../../src/Yaml.h \
	../../src/YamlEmitter.h \
	../../src/YamlParser.h \
	../../src/dialogs/EditProfileDialog.h \
	
SOURCES += \
	../../src/ClickableLabel.cpp \
	../../src/ConsoleString.cpp \
	../../src/FileList.cpp \
	../../src/FoldedHeader.cpp \
	../../src/HintLineEdit.cpp \
	../../src/LocalScript.cpp \
    ../../src/NoSlashValidator.cpp \
	../../src/ProfileManager.cpp \
	../../src/Proteomatic.cpp \
	../../src/RemoteScript.cpp \
	../../src/RubyWindow.cpp \
	../../src/Script.cpp \
	../../src/ScriptFactory.cpp \
    ../../src/StopWatch.cpp \
	../../src/TicketWindow.cpp \
	../../src/UnclickableLabel.cpp \
	../../src/version.cpp \
	../../src/Yaml.cpp \
	../../src/YamlEmitter.cpp \
	../../src/YamlParser.cpp \
	../../src/dialogs/EditProfileDialog.cpp \
	
RESOURCES += \
	../../src/Proteomatic.qrc \

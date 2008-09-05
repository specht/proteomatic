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

QT = core gui network

# libyaml files
HEADERS += ../../src/libyaml/ly_config.h \
           ../../src/libyaml/ly_yaml.h \ 
           ../../src/libyaml/ly_yaml_private.h \
           
SOURCES += ../../src/libyaml/ly_api.c \
           ../../src/libyaml/ly_dumper.c \
           ../../src/libyaml/ly_emitter.c \
           ../../src/libyaml/ly_loader.c \
           ../../src/libyaml/ly_parser.c \
           ../../src/libyaml/ly_reader.c \
           ../../src/libyaml/ly_scanner.c \
           ../../src/libyaml/ly_writer.c \

INCLUDEPATH += ../../src/libyaml/ ../../src/ ../../src/dialogs

# Input files
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
    ../../src/StopWatch.h \
	../../src/TicketWindow.h \
	../../src/Yaml.h \
	../../src/YamlEmitter.h \
	../../src/YamlParser.h \
	../../src/dialogs/EditProfileDialog.h \
	
SOURCES += \
	../../src/ClickableLabel.cpp \
	../../src/ConsoleString.cpp \
	../../src/FileList.cpp \
	../../src/FoldedHeader.cpp \
	../../src/LocalScript.cpp \
	../../src/ProfileManager.cpp \
	../../src/Proteomatic.cpp \
	../../src/RemoteScript.cpp \
	../../src/RubyWindow.cpp \
	../../src/Script.cpp \
	../../src/ScriptFactory.cpp \
	../../src/ScriptHelper.cpp \
    ../../src/StopWatch.cpp \
	../../src/TicketWindow.cpp \
	../../src/Yaml.cpp \
	../../src/YamlEmitter.cpp \
	../../src/YamlParser.cpp \
	../../src/dialogs/EditProfileDialog.cpp \
	
RESOURCES += \
	../../src/Proteomatic.qrc \

TEMPLATE = app

win32 {
	INCLUDEPATH += c:/Qt/2009.04/mingw/include
}

!isEmpty(PROTEOMATIC_UPDATES_ENABLED) {
    message("updates enabled!")
    DEFINES += PROTEOMATIC_UPDATES_ENABLED
}

CONFIG += debug_and_release

macx {
	CONFIG += app_bundle
	CONFIG += x86 ppc
	QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.4
	ICON = src/icons/proteomatic-pipeline.icns
	INCLUDEPATH += '/Users/michael/programming/ext/include'
	INCLUDEPATH += '/Users/dimitris/michael/ext/include'
}

CONFIG(debug, debug|release) {
	OBJECTS_DIR = obj/debug/
	MOC_DIR = obj/debug/
	RCC_DIR = obj/debug/
	TARGET = $$join(TARGET,,,_debug)
	DEFINES += DEBUG
}
else {
	OBJECTS_DIR = obj/release/
	MOC_DIR = obj/release/
	RCC_DIR = obj/release/
}

QT = core gui webkit

INCLUDEPATH += src/ src/dialogs

win32 {
    LIBPATH += c:\Qt\2009.04\mingw\lib\
}
macx {
    LIBPATH += /Users/michael/programming/ext/lib
    LIBPATH += /Users/dimitris/michael/ext/lib
}

#LIBS += $$MYLIBPATH/libyaml-cpp.a $$MYLIBPATH/libmd5.a
LIBS += -lyaml-cpp -lmd5

TARGET = Proteomatic
macx {
    TARGET = Proteomatic
    QMAKE_INFO_PLIST    = Info.plist
}

!isEmpty(PROTEOMATIC_UPDATES_ENABLED) {
    DESTDIR = bin/
    TARGET = ProteomaticCore
}

include(../base.pro)


win32 {
    RC_FILE = Proteomatic.rc
}


# Input files
HEADERS += \
	src/CiListWidgetItem.h \
	src/ClickableGraphicsProxyWidget.h \
	src/ClickableLabel.h \
	src/ConsoleString.h \
	src/FileList.h \
	src/FoldedHeader.h \
	src/HintLineEdit.h \
	src/IScript.h \
	src/LocalScript.h \
    src/NoSlashValidator.h \
	src/ProfileManager.h \
	src/Proteomatic.h \
	src/RefPtr.h \
	src/RemoteScript.h \
	src/RubyWindow.h \
	src/Script.h \
	src/ScriptFactory.h \
    src/StopWatch.h \
	src/Tango.h \
	src/TicketWindow.h \
	src/UnclickableLabel.h \
	src/version.h \
	src/Yaml.h \
	src/YamlEmitter.h \
	src/YamlParser.h \
	src/dialogs/EditProfileDialog.h \
    src/Desktop.h \
    src/DesktopBox.h \
    src/DesktopBoxFactory.h \
    src/IDesktopBox.h \
    src/IFileBox.h \
    src/InputGroupProxyBox.h \
    src/IScriptBox.h \
    src/FileListBox.h \
    src/OutFileListBox.h \
    src/PipelineMainWindow.h \
    src/ScriptBox.h \
    src/ZoomableWebView.h \
	
SOURCES += \
    src/ClickableGraphicsProxyWidget.cpp \
	src/ClickableLabel.cpp \
	src/ConsoleString.cpp \
	src/FileList.cpp \
	src/FoldedHeader.cpp \
	src/HintLineEdit.cpp \
	src/LocalScript.cpp \
    src/NoSlashValidator.cpp \
	src/ProfileManager.cpp \
	src/Proteomatic.cpp \
	src/RemoteScript.cpp \
	src/RubyWindow.cpp \
	src/Script.cpp \
	src/ScriptFactory.cpp \
    src/StopWatch.cpp \
	src/TicketWindow.cpp \
	src/UnclickableLabel.cpp \
	src/version.cpp \
	src/Yaml.cpp \
	src/YamlEmitter.cpp \
	src/YamlParser.cpp \
	src/dialogs/EditProfileDialog.cpp \
    src/Desktop.cpp \
    src/DesktopBox.cpp \
    src/DesktopBoxFactory.cpp \
    src/FileListBox.cpp \
    src/InputGroupProxyBox.cpp \
    src/OutFileListBox.cpp \
    src/PipelineMainWindow.cpp \
    src/ProteomaticPipelineMain.cpp \
    src/ScriptBox.cpp \
    src/ZoomableWebView.cpp \
	
RESOURCES += \
	src/Proteomatic.qrc \

isEmpty( PREFIX ) {
    PREFIX = /usr
    count( INSTALLDIR, 1 ) {
        PREFIX = $${INSTALLDIR}
        message( "Please use PREFIX instead of INSTALLDIR" )
    }
}
isEmpty( BINDIR ) {
    BINDIR = $${PREFIX}/bin
}
isEmpty( DATADIR ) {
    DATADIR = $${PREFIX}/share
}
isEmpty( DOCDIR ) {
    DOCDIR = $${DATADIR}/doc/packages/proteomatic
}

support.files = src/icons/proteomatic-pipeline.png
support.path = $${DATADIR}/proteomatic
INSTALLS += support 

target.path = $${BINDIR}
INSTALLS += target

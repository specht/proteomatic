TEMPLATE = app

win32 {
    INCLUDEPATH += e:/Qt/2010.02.1/mingw/include
}

!isEmpty(PROTEOMATIC_UPDATES_ENABLED) {
    message("BUILD FLAG: updates enabled!")
    DEFINES += PROTEOMATIC_UPDATES_ENABLED
}

!isEmpty(PROTEOMATIC_PORTABLE) {
    message("BUILD FLAG: portable!")
    DEFINES += PROTEOMATIC_PORTABLE
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

TARGET = Proteomatic

CONFIG(debug, debug|release) {
    DEFINES += DEBUG
    OBJECTS_DIR = obj/debug/
    MOC_DIR = obj/debug/
    RCC_DIR = obj/debug/
    TARGET = $$join(TARGET,,,_debug)
}
else {
    OBJECTS_DIR = obj/release/
    MOC_DIR = obj/release/
    RCC_DIR = obj/release/
}

QT = core gui

INCLUDEPATH += src/ src/dialogs

win32 {
    LIBPATH += e:\Qt\2010.02.1\mingw\lib\
}
macx {
    LIBPATH += /Users/michael/programming/ext/lib
    LIBPATH += /Users/dimitris/michael/ext/lib
}

#LIBS += $$MYLIBPATH/libyaml-cpp.a $$MYLIBPATH/libmd5.a
LIBS += -lyaml-cpp -lmd5

macx {
    TARGET = Proteomatic
    QMAKE_INFO_PLIST    = Info.plist
}

DESTDIR = ./

!isEmpty(PROTEOMATIC_UPDATES_ENABLED) {
    DESTDIR = bin/
    TARGET = ProteomaticCore
}


win32 {
    RC_FILE = Proteomatic.rc
}


# Input files
HEADERS += \
    src/CiListWidgetItem.h \
    src/ClickableGraphicsProxyWidget.h \
    src/ClickableLabel.h \
    src/ConsoleString.h \
    src/Desktop.h \
    src/DesktopBox.h \
    src/DesktopBoxFactory.h \
    src/FileList.h \
    src/FileListBox.h \
    src/FoldedHeader.h \
    src/HintLineEdit.h \
    src/IDesktopBox.h \
    src/IFileBox.h \
    src/InputGroupProxyBox.h \
    src/IScript.h \
    src/IScriptBox.h \
    src/LocalScript.h \
    src/NoSlashValidator.h \
    src/PipelineMainWindow.h \
    src/ProfileManager.h \
    src/Proteomatic.h \
    src/RemoteScript.h \
    src/RubyWindow.h \
    src/Script.h \
    src/ScriptBox.h \
    src/ScriptFactory.h \
    src/StopWatch.h \
    src/Tango.h \
    src/TicketWindow.h \
    src/UnclickableLabel.h \
    src/Yaml.h \
    src/YamlEmitter.h \
    src/YamlParser.h \
#     src/ZoomableWebView.h \
    src/version.h \
    src/dialogs/EditProfileDialog.h \
    
SOURCES += \
    src/ClickableGraphicsProxyWidget.cpp \
    src/ClickableLabel.cpp \
    src/ConsoleString.cpp \
    src/Desktop.cpp \
    src/DesktopBox.cpp \
    src/DesktopBoxFactory.cpp \
    src/FileList.cpp \
    src/FileListBox.cpp \
    src/FoldedHeader.cpp \
    src/HintLineEdit.cpp \
    src/InputGroupProxyBox.cpp \
    src/LocalScript.cpp \
    src/NoSlashValidator.cpp \
    src/PipelineMainWindow.cpp \
    src/ProfileManager.cpp \
    src/Proteomatic.cpp \
    src/ProteomaticPipelineMain.cpp \
    src/RemoteScript.cpp \
    src/RubyWindow.cpp \
    src/Script.cpp \
    src/ScriptBox.cpp \
    src/ScriptFactory.cpp \
    src/StopWatch.cpp \
    src/TicketWindow.cpp \
    src/UnclickableLabel.cpp \
    src/version.cpp \
    src/Yaml.cpp \
    src/YamlEmitter.cpp \
    src/YamlParser.cpp \
#     src/ZoomableWebView.cpp \
    src/dialogs/EditProfileDialog.cpp \
    
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

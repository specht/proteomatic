TEMPLATE = app

win32 {
	INCLUDEPATH += c:/Qt/2009.04/mingw/include
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

win32 {
    DESTDIR = bin/
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

include(../base.pro)


win32 {
    RC_FILE = Proteomatic.rc
    TARGET = ProteomaticCore
}


# Input files
HEADERS += \
	src/RefPtr.h \
	src/version.h \
    src/PipelineMainWindow.h \
	
SOURCES += \
    src/PipelineMainWindow.cpp \
    src/ProteomaticPipelineMain.cpp \
	
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

message( "Installation directory" )
message( $$PREFIX )


support.files = src/icons/proteomatic-pipeline.png
support.path = $${DATADIR}/proteomatic
INSTALLS += support 

target.path = $${BINDIR}
INSTALLS += target

TARGET = bin/Revelio

include(../base.pro)

win32 {
    RC_FILE = Revelio.rc
}

# Input
HEADERS += \
	../../src/RevelioMainWindow.h \
	../../src/Surface.h \
	../../src/FileTrackerNode.h \
	
SOURCES += \
	../../src/RevelioMainWindow.cpp \
	../../src/RevelioMain.cpp \
	../../src/Surface.cpp \
	../../src/FileTrackerNode.cpp \
	
RESOURCES += \
	../../src/Proteomatic.qrc \
	
QT += sql

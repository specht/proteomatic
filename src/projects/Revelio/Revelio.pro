TARGET = Revelio

include(../base.pro)

win32 {
    RC_FILE = Revelio.rc
}

# Input
HEADERS += \
	../../src/RevelioMainWindow.h \

SOURCES += \
	../../src/RevelioMainWindow.cpp \
	../../src/RevelioMain.cpp \
	
RESOURCES += \
	../../src/Proteomatic.qrc \

TARGET = bin/Proteomatic

include(../base.pro)

win32 {
    RC_FILE = Proteomatic.rc
}

HEADERS += \
	../../src/ScriptHelper.h \
	
SOURCES += \
	../../src/ProteomaticMain.cpp \
	../../src/ScriptHelper.cpp \

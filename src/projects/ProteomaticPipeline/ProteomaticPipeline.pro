TARGET = ProteomaticPipeline

include(../base.pro)

# Input
HEADERS += \
    ../../src/Desktop.h \
    ../../src/DesktopBox.h \
	../../src/PipelineMainWindow.h \

SOURCES += \
    ../../src/Desktop.cpp \
    ../../src/DesktopBox.cpp \
	../../src/PipelineMainWindow.cpp \
	../../src/ProteomaticPipelineMain.cpp \
	
RESOURCES += \
	../../src/Proteomatic.qrc \

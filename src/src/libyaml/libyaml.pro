TEMPLATE = lib
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .

CONFIG = staticlib release

# Input
HEADERS += config.h \
           yaml.h \ 
           yaml_private.h \
           
SOURCES += api.c \
           dumper.c \
           emitter.c \
           loader.c \
           parser.c \
           reader.c \
           scanner.c \
           writer.c \


#--------------------------------------------------------
#
# ChartGeany tool: a command line tool for ChartGeany
#
# ChartGeany: chartgeany@gmail.com
#
#--------------------------------------------------------

QT       += core
QT       -= gui
DEFINES  *= QT_NO_DEBUG_OUTPUT
DEFINES  *= QT_NO_WARNING_OUTPUT
DEFINES  *= QT_USE_QSTRINGBUILDER
TEMPLATE  = app
TARGET    = cgtool
PKG_NAME  = chartgeany
OBJECTS_DIR = obj
RESOURCES += cgtool.qrc

CONFIG += release cmdline console c99 strict_c  c++11 strict_c++

QMAKE_CXXFLAGS += -DCGTOOL

TCC_ARCH    = TCC_TARGET_I386

#
# Qt4
equals(QT_MAJOR_VERSION, 4) {
CONFIG += qt  
unix {
QMAKE_CFLAGS   += -std=c99
QMAKE_CXXFLAGS += -std=c++11
}
*g++:QMAKE_CXXFLAGS +=-Wno-deprecated-copy
}

INCLUDEPATH = include \
              ../ChartGeany/3rdparty/sqlite3/include \
              ../ChartGeany/3rdparty/simplecrypt/include \
              ../ChartGeany/include \
              ../ChartGeany/cgscript/include

*g++|*clang {
QMAKE_LFLAGS += -fstack-protector
QMAKE_CFLAGS += -w -fomit-frame-pointer -fstack-protector -fexceptions
QMAKE_CXXFLAGS += -fomit-frame-pointer -fstrict-aliasing
QMAKE_CXXFLAGS += -fstrict-enums -pedantic
QMAKE_CXXFLAGS += -fomit-frame-pointer -fstack-protector
#QMAKE_CXXFLAGS += -g -O0
}

linux* {
LIBS += -ldl
}

macx* {
LIBS += -framework CoreServices
equals(QT_MAJOR_VERSION, 4) {
clang:QMAKE_CXXFLAGS += -stdlib=libc++ -mmacosx-version-min=10.7 -std=c++11
clang:QMAKE_LFLAGS += -mmacosx-version-min=10.7
clang:LIBS += -stdlib=libc++
}
}

solaris* {
QMAKE_CFLAGS 	+= -D_XPG6 -D$$TCC_ARCH -D__EXTENSIONS__
solaris-cc {
QMAKE_LFLAGS	+= -library=CrunG3 -std=c++11
}
}

unix {
QMAKE_STRIP = echo
QMAKE_POST_LINK += strip $(TARGET)
target.path = /opt/$$PKG_NAME
INSTALLS += target
}

SOURCES += src/cgscript_toolchain.cpp \
           src/compile_module.cpp \
           src/decompile_module.cpp \
           src/control.cpp \
           src/dbfile_info.cpp \
           src/delete_module.cpp \
           src/export_data.cpp \
           src/export_module.cpp \
           src/help.cpp \
           src/import_module.cpp \
           src/list_modules.cpp \
           src/list_symbols.cpp \
           src/main.cpp \
           src/updatedb.cpp \
           ../ChartGeany/src/asprintf.c \
	   ../ChartGeany/src/atomic.c \
           ../ChartGeany/src/platformstring.cpp \
           ../ChartGeany/src/selectfromdb.cpp \
           ../ChartGeany/src/sqlcb_datafeeds.cpp \
           ../ChartGeany/src/sqlcb_dbversion.cpp \
           ../ChartGeany/src/sqlcb_modules.cpp \
           ../ChartGeany/src/sqlcb_symbol_table.cpp \
           ../ChartGeany/src/sqlcb_toolchain.cpp \
           ../ChartGeany/3rdparty/simplecrypt/simplecrypt.cpp \
           ../ChartGeany/3rdparty/sqlite3/sqlite3_wrapper.c

HEADERS += ../ChartGeany/include/appdata.h \
           ../ChartGeany/3rdparty/simplecrypt/include/simplecrypt.h

#
# libtcc
linux*|win32*|solaris* {
INCLUDEPATH += \
        ../ChartGeany/3rdparty/libtcc/include \
        ../ChartGeany/3rdparty/libtcc/i386 \
        ../ChartGeany/3rdparty/libtcc/x86_64

SOURCES += \
        ../ChartGeany/3rdparty/libtcc/src/tccasm.c \
        ../ChartGeany/3rdparty/libtcc/src/tccelf.c \
        ../ChartGeany/3rdparty/libtcc/src/tccgen.c \
        ../ChartGeany/3rdparty/libtcc/src/tccpp.c \
        ../ChartGeany/3rdparty/libtcc/src/tccrun.c \
        ../ChartGeany/3rdparty/libtcc/src/libtcc.c
}

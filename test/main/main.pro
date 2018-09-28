include( ../../pri/nbody.pri )
include( ../../pri/opencl.pri )
include( ../../pri/vectorize.pri )

TEMPLATE	= app
TARGET		= main
MOC_DIR = ./.tmp/moc

CONFIG		+= qt
QT += opengl
LIBS += -lGLU

INCLUDEPATH += ../../nbody
LIBS += -L../../nbody
LIBS += -lnbody

SOURCES	+= main.cpp



QT  += core gui
QT  -= sql
QT  += widgets

TARGET = edbee-app-test
TEMPLATE = app

EDBEE_SANITIZE = $$(EDBEE_SANITIZE)
!isEmpty( EDBEE_SANITIZE ) {
  warning('*** SANITIZE ENABLED! edbee-app-test ***')
  QMAKE_CXXFLAGS+=-fsanitize=address -fsanitize=bounds -fsanitize-undefined-trap-on-error
  QMAKE_LFLAGS+=-fsanitize=address -fsanitize=bounds -fsanitize-undefined-trap-on-error
}

# This seems to be required for Windows
INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
DEFINES += QT_NODLL


# The test sources
SOURCES += \
	main.cpp \
    models/edbeeconfigtest.cpp


# manually add the sources that need testing
SOURCES += \
	..\edbee-app\models\edbeeconfig.cpp



HEADERS += \
	models/edbeeconfigtest.h

INCLUDEPATH += $$PWD/../edbee-app

## Extra dependencies
##====================
include(../vendor/qslog/QsLog.pri)


## edbee-lib dependency
##=======================

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../edbee-lib/edbee-lib/release/ -ledbee
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../edbee-lib/edbee-lib/debug/ -ledbee
else:unix:!symbian: LIBS += -L$$OUT_PWD/../edbee-lib/edbee-lib// -ledbee

INCLUDEPATH += $$PWD/../edbee-lib/edbee-lib
DEPENDPATH += $$PWD/../edbee-lib/edbee-lib


win32-msvc*:LIBNAME=edbee.lib
else:LIBNAME=libedbee.a

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../edbee-lib/edbee-lib//release/$$LIBNAME
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../edbee-lib/edbee-lib/debug/$$LIBNAME
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../edbee-lib/edbee-lib/$$LIBNAME





# This example demonstrates the use of the edbee component as a library dependency

include($$PWD/edbee-app.pri)

EDBEE_SANITIZE = $$(EDBEE_SANITIZE)
!isEmpty( EDBEE_SANITIZE ) {
  QMAKE_CXXFLAGS+=-fsanitize=address -fsanitize=bounds -fsanitize-undefined-trap-on-error
  QMAKE_LFLAGS+=-fsanitize=address -fsanitize=bounds -fsanitize-undefined-trap-on-error
}


## edbee-lib dependency
##=======================

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../edbee-lib/edbee-lib/release/ -ledbee
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../edbee-lib/edbee-lib/debug/ -ledbee
else:unix:!symbian: LIBS += -L$$OUT_PWD/../edbee-lib/edbee-lib/ -ledbee

INCLUDEPATH += $$PWD/../edbee-lib/edbee-lib
DEPENDPATH += $$PWD/../edbee-lib/edbee-lib


win32-msvc*:LIBNAME=edbee.lib
else:LIBNAME=libedbee.a

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../edbee-lib/edbee-lib/release/$$LIBNAME
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../edbee-lib/edbee-lib/debug/$$LIBNAME
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../edbee-lib/edbee-lib/$$LIBNAME




# This example demonstrates the use of the edbee component as a library dependency

include($$PWD/edbee-app.pri)

#QMAKE_CXXFLAGS+=-fsanitize=address -fsanitize=bounds
#QMAKE_LFLAGS+=-fsanitize=address -fsanitize=bounds

## edbee-lib dependency
##=======================

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../edbee-lib/edbee-lib/release/ -ledbee
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../edbee-lib/edbee-lib/debug/ -ledbee
else:unix:!symbian: LIBS += -L$$OUT_PWD/../edbee-lib/edbee-lib/ -ledbee

INCLUDEPATH += $$PWD/../edbee-lib/edbee-lib/
DEPENDPATH += $$PWD/../edbee-lib/edbee-lib/

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../edbee-lib/edbee-lib/release/edbee.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../edbee-lib/edbee-lib/debug/edbee.lib
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../edbee-lib/edbee-lib/libedbee.a



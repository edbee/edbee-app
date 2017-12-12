TEMPLATE = subdirs

# DEFINE 'EDBEE_SANITIZE' to enable santitize bounds checks
EDBEE_SANITIZE = $$(EDBEE_SANITIZE)
!isEmpty( EDBEE_SANITIZE ) {
  warning('*** SANITIZE ENABLED! edbee-app (global) ***')
  QMAKE_CXXFLAGS+=-fsanitize=address -fsanitize=bounds -fsanitize-undefined-trap-on-error
  QMAKE_LFLAGS+=-fsanitize=address -fsanitize=bounds -fsanitize-undefined-trap-on-error


}

src_lib.subdir = edbee-lib

#src_lib_test.subdir = edbee-test
#src_lib_test.depends = src_lib

src_app.subdir = edbee-app
src_app.depends = src_lib

src_app_test.subdir = edbee-app-test
src_app_test.depends = src_lib src_app

#SUBDIRS = \
#	src_lib \
#	src_lib_test \
#	src_app \
#	src_app_test

SUBDIRS = \
	src_lib \
	src_app \
	src_app_test


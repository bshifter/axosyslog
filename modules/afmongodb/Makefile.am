
if ENABLE_MONGODB

module_LTLIBRARIES 				+= modules/afmongodb/libafmongodb.la

modules_afmongodb_libafmongodb_la_CFLAGS 	=	\
    $(AM_CFLAGS) \
	$(LIBMONGO_CFLAGS) 				\
	-I$(top_srcdir)/modules/afmongodb		\
	-I$(top_builddir)/modules/afmongodb
modules_afmongodb_libafmongodb_la_SOURCES	=	\
	modules/afmongodb/afmongodb-grammar.y		\
	modules/afmongodb/afmongodb.c			\
	modules/afmongodb/afmongodb.h			\
	modules/afmongodb/afmongodb-worker.c			\
	modules/afmongodb/afmongodb-worker.h			\
	modules/afmongodb/afmongodb-private.h			\
	modules/afmongodb/afmongodb-parser.c		\
	modules/afmongodb/afmongodb-parser.h		\
	${DUMMY_C}

modules_afmongodb_libafmongodb_la_LIBADD	=	\
	$(MODULE_DEPS_LIBS) $(LIBMONGO_LIBS)
modules_afmongodb_libafmongodb_la_LDFLAGS	=	\
	$(MODULE_LDFLAGS)
EXTRA_modules_afmongodb_libafmongodb_la_DEPENDENCIES	=	\
	$(MODULE_DEPS_LIBS) $(lmc_EXTRA_DEPS)

modules/afmongodb modules/afmongodb/ mod-afmongodb mod-mongodb: \
	modules/afmongodb/libafmongodb.la

include modules/afmongodb/tests/Makefile.am
else
modules/afmongodb modules/afmongodb/ mod-afmongodb mod-mongodb:
endif

BUILT_SOURCES					+=	\
	modules/afmongodb/afmongodb-grammar.y		\
	modules/afmongodb/afmongodb-grammar.c		\
	modules/afmongodb/afmongodb-grammar.h

EXTRA_DIST					+=	\
	modules/afmongodb/afmongodb-grammar.ym \
	modules/afmongodb/CMakeLists.txt \
	modules/afmongodb/tests/CMakeLists.txt


.PHONY: modules/afmongodb/ mod-afmongodb mod-mongodb

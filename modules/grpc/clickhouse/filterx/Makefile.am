if ENABLE_GRPC

# noinst: Built as part of a larger libclickhouse module as static library
noinst_LTLIBRARIES += modules/grpc/clickhouse/filterx/libfilterx.la

modules_grpc_clickhouse_filterx_libfilterx_la_SOURCES = \
  modules/grpc/clickhouse/filterx/object-protobuf.h \
  modules/grpc/clickhouse/filterx/object-protobuf-message.cpp \
  modules/grpc/clickhouse/filterx/object-protobuf-message.hpp

modules_grpc_clickhouse_filterx_libfilterx_la_CXXFLAGS = \
  $(AM_CXXFLAGS) \
  $(PROTOBUF_CFLAGS) \
  $(GRPCPP_CFLAGS) \
  -I$(GOOGLEAPIS_PROTO_BUILDDIR) \
  -I$(top_srcdir)/modules/grpc/clickhouse/filterx \
  -I$(top_builddir)/modules/grpc/clickhouse/filterx \
  -Wno-deprecated-declarations

modules_grpc_clickhouse_filterx_libfilterx_la_LIBADD = \
  $(MODULE_DEPS_LIBS) \
  $(PROTOBUF_LIBS) \
  $(top_builddir)/modules/grpc/protos/libgrpc-protos.la

modules_grpc_clickhouse_filterx_libfilterx_la_LDFLAGS = $(MODULE_LDFLAGS)

EXTRA_modules_grpc_clickhouse_filterx_libfilterx_la_DEPENDENCIES = \
  $(MODULE_DEPS_LIBS) \
  $(top_builddir)/modules/grpc/protos/libgrpc-protos.la

endif

EXTRA_DIST += \
  modules/grpc/clickhouse/filterx/CMakeLists.txt

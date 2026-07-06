cmake_minimum_required(VERSION 3.7)

find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

get_target_property(GRPC_CPP_PLUGIN gRPC::grpc_cpp_plugin LOCATION)

set(PROTO_SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}/proto)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

set(PROTO_SRCS ${GENERATED_DIR}/rating.pb.cc)
set(PROTO_HDRS ${GENERATED_DIR}/rating.pb.h)
set(GRPC_SRCS  ${GENERATED_DIR}/rating.grpc.pb.cc)
set(GRPC_HDRS  ${GENERATED_DIR}/rating.grpc.pb.h)

add_custom_command(
    OUTPUT ${PROTO_SRCS} ${PROTO_HDRS} ${GRPC_SRCS} ${GRPC_HDRS}
    COMMAND $<TARGET_FILE:protobuf::protoc>
    ARGS
        --grpc_out=${GENERATED_DIR}
        --cpp_out=${GENERATED_DIR}
        --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
        -I${PROTO_SRCDIR}
        ${PROTO_SRCDIR}/rating.proto
    DEPENDS ${PROTO_SRCDIR}/rating.proto
    COMMENT "Compiling rating.proto"
)

add_custom_target(grpcapi_proto_gen
    DEPENDS ${PROTO_SRCS} ${PROTO_HDRS} ${GRPC_SRCS} ${GRPC_HDRS}
)

include(CMakeLists.var)
include("${CMAKE_SOURCE_DIR}/cmake/BaseModule.cmake")

# Append generated proto/gRPC sources and expose the generated include dir.
target_sources(${project_name}_${module_name} PRIVATE ${PROTO_SRCS} ${GRPC_SRCS})
target_include_directories(${project_name}_${module_name} PUBLIC ${GENERATED_DIR})
add_dependencies(${project_name}_${module_name} grpcapi_proto_gen)

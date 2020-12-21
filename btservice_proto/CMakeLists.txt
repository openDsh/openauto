include(FindProtobuf)
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})

file(GLOB_RECURSE proto_files ${CMAKE_CURRENT_SOURCE_DIR}/*.proto)
protobuf_generate_cpp(proto_sources proto_headers ${proto_files})
add_library(btservice_proto SHARED ${proto_headers} ${proto_sources})
target_link_libraries(btservice_proto ${PROTOBUF_LIBRARIES})

install(TARGETS btservice_proto DESTINATION lib)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} DESTINATION include
        FILES_MATCHING PATTERN *.h
        PATTERN CMakeFiles EXCLUDE)

add_executable(stream_server stream_server.c)
target_link_libraries(stream_server yolanda)

add_executable(stream_client stream_client.c)
target_link_libraries(stream_client yolanda)

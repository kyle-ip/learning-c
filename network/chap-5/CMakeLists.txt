add_executable(tcp_client tcp_client.c)
target_link_libraries(tcp_client yolanda)

add_executable(tcp_server tcp_server.c)
target_link_libraries(tcp_server yolanda)


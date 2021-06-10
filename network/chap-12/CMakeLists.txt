add_executable(ping_client ping_client.c)
target_link_libraries(ping_client yolanda)

add_executable(ping_server ping_server.c)
target_link_libraries(ping_server yolanda)

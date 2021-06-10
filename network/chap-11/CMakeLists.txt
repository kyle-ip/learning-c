add_executable(grace_client grace_client.c)
target_link_libraries(grace_client yolanda)

add_executable(grace_server grace_server.c)
target_link_libraries(grace_server yolanda)


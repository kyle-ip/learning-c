add_executable(http_server01 http_server01.c)
target_link_libraries(http_server01 yolanda)

add_executable(http_server_test http_server_test.c)
target_link_libraries(http_server_test yolanda)

add_executable(telnet_client telnet_client.c)
target_link_libraries(telnet_client yolanda)

add_executable(telnet_server telnet_server.c)
target_link_libraries(telnet_server yolanda)

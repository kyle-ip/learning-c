project (test C)

add_library(common common.c)

add_executable(server server.c)
#target_link_libraries(server server)

add_executable(client client.c)
#target_link_libraries(client test)
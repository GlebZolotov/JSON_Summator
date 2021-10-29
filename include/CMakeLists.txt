add_library(include_lib INTERFACE)
target_include_directories(include_lib INTERFACE ./)

add_library(p7headers_lib INTERFACE)
target_include_directories(p7headers_lib INTERFACE ./P7_Headers)
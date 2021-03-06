project(tawashi VERSION 0.2.3 LANGUAGES CXX C)

option(TAWASHI_WITH_IP_LOGGING "Enable code in Tawashi that may result in users IPs being stored in the DB or in logs" ON)

find_package(Boost 1.53.0 REQUIRED)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME}
	split_get_vars.cpp
	cgi_environment_vars.cpp
	cgi_env.cpp
	cgi_post.cpp
	escapist.cpp
	sanitized_utf8.cpp
	tiger.c
	tawashi_exception.cpp
	http_header.cpp
	ip_utils.cpp
	mime_split.cpp
	truncated_string.cpp
	version.cpp
)

target_include_directories(${PROJECT_NAME}
	PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include
	PUBLIC ${TAWASHI_SOURCE_ROOT}/lib/kakoune
	PUBLIC ${TAWASHI_SOURCE_ROOT}/lib/mstch/include
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
target_include_directories(${PROJECT_NAME} SYSTEM
	PUBLIC ${Boost_INCLUDE_DIRS}
	PUBLIC ${TAWASHI_SOURCE_ROOT}/lib/better-enums
	PRIVATE ${SourceHighlight_INCLUDE_DIR}
	PRIVATE ${TAWASHI_SOURCE_ROOT}/lib/utf8_v2_3_4/source
	PUBLIC ${TAWASHI_SOURCE_ROOT}/lib/spdlog/include
)
target_link_libraries(${PROJECT_NAME}
	PRIVATE ${Boost_LIBRARIES}
	PRIVATE houdini
	PRIVATE duckhandy
)
target_compile_definitions(${PROJECT_NAME}
	PRIVATE BOOST_SPIRIT_USE_PHOENIX_V3=1
	PUBLIC $<$<CONFIG:Debug>:SPDLOG_DEBUG_ON>
	PUBLIC $<$<CONFIG:Debug>:SPDLOG_TRACE_ON>
	PUBLIC $<$<BOOL:${BUILD_TESTING}>:tawashi_virtual_testing=virtual>
	PUBLIC $<$<NOT:$<BOOL:${BUILD_TESTING}>>:tawashi_virtual_testing=>
	PUBLIC $<$<BOOL:${BUILD_TESTING}>:TAWASHI_WITH_TESTING>
)
target_compile_options(${PROJECT_NAME}
	PRIVATE -fdiagnostics-color=always
)

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/tawashi_config.h.in"
	"${CMAKE_CURRENT_BINARY_DIR}/include/tawashi_config.h"
)

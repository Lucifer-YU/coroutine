cmake_minimum_required (VERSION 2.6)
project (coroutine)

if (CMAKE_BUILD_TYPE)
else()
    set(CMAKE_BUILD_TYPE RELEASE)
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

# Display verbose info
set (CMAKE_VERBOSE_MAKEFILE ON)

# Setting up version numbers
set (VERSION_MAJOR 1)
set (VERSION_MINOR 0)

# Options
option(ENABLE_SYS_HOOKS "Enable system API hooks" OFF)
message(STATUS "ENABLE_SYS_HOOKS: ${ENABLE_SYS_HOOKS}")

if (UNIX)
    # Select coroutine context.
    set(USE_UCONTEXT 1)
    message (STATUS "Use ucontext context")

    # Select the polling method
    set (POLLER "" CACHE STRING "Choose polling method. valid values are kqueue, epoll or poll [default=autodetect]")
    include (CheckFunctionExists)
    if (POLLER STREQUAL "")
        set (CMAKE_REQUIRED_INCLUDES sys/event.h)
        check_function_exists (kqueue HAVE_KQUEUE)
        set (CMAKE_REQUIRED_INCLUDES)
        if (HAVE_KQUEUE)
            set (POLLER "kqueue")
        endif ()
    endif ()
    if (POLLER STREQUAL "")
        set (CMAKE_REQUIRED_INCLUDES sys/epoll.h)
        check_function_exists (epoll_create HAVE_EPOLL)
        set (CMAKE_REQUIRED_INCLUDES)
        if (HAVE_EPOLL)
            set (POLLER "epoll")
        endif ()
    endif ()
    if (POLLER STREQUAL "")
        set (CMAKE_REQUIRED_INCLUDES sys/poll.h)
        check_function_exists (poll HAVE_POLL)
        set (CMAKE_REQUIRED_INCLUDES)
        if (HAVE_POLL)
            set (POLLER "poll")
        else ()
            message (FATAL_ERROR, "Could not autodetect polling method")
        endif ()
    endif ()
    message (STATUS "Detected ${POLLER} polling method")
    string (TOUPPER ${POLLER} UPPER_POLLER)
    set (USE_${UPPER_POLLER} 1)

elseif (WIN32)
    # Select coroutine context.
    set(USE_FIBER 1)
    message (STATUS "Use fiber context")
endif ()

# Configuration files
configure_file (
    ${CMAKE_SOURCE_DIR}/config.h.in
    ${PROJECT_BINARY_DIR}/config.h
)

set (CMAKE_SOURCE_DIR .)
if (UNIX)
	set (CMAKE_C_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb ") # –pg
    set (CMAKE_C_FLAGS_RELEASE "$ENV{CXXFLAGS} -DNDEBUG -O3 -Wall ")
elseif (WIN32)
    ###add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set (CMAKE_C_FLAGS_DEBUG "-D_CRT_SECURE_NO_WARNINGS ")
    set (CMAKE_C_FLAGS_RELEASE "-D_CRT_SECURE_NO_WARNINGS -DNDEBUG ")
endif ()
set (CMAKE_CXX_STANDARD 11)

# Header & Source files
include_directories (
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${PROJECT_BINARY_DIR}
)
aux_source_directory(${CMAKE_SOURCE_DIR}/src coroutine_SRCS)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/dbg coroutine_SRCS)
if (UNIX)
    include_directories(${CMAKE_SOURCE_DIR}/src/unix)
    aux_source_directory(${CMAKE_SOURCE_DIR}/src/unix coroutine_SRCS)
elseif (WIN32)
    include_directories(${CMAKE_SOURCE_DIR}/src/win32)
    aux_source_directory(${CMAKE_SOURCE_DIR}/src/win32 coroutine_SRCS)
endif ()

if (ENABLE_SYS_HOOKS)
    include_directories(${CMAKE_SOURCE_DIR}/src/hooks)
    aux_source_directory(${CMAKE_SOURCE_DIR}/src/hooks coroutine_SRCS)
    if (UNIX)
        include_directories(${CMAKE_SOURCE_DIR}/src/hooks/unix)
        aux_source_directory(${CMAKE_SOURCE_DIR}/src/hooks/unix coroutine_SRCS)
    elseif (WIN32)
        include_directories(${CMAKE_SOURCE_DIR}/src/hooks/win32)
        aux_source_directory(${CMAKE_SOURCE_DIR}/src/hooks/win32 coroutine_SRCS)
    endif ()
endif ()

# The targets
set(TARGET co)
add_library(${TARGET} STATIC ${coroutine_SRCS})

# Macro for add example target
macro(add_example_target EXAMPLE_TARGET)
#    add_executable("example_${EXAMPLE_TARGET}" "examples/example_${EXAMPLE_TARGET}.c")
#    target_link_libraries("example_${EXAMPLE_TARGET}" co)
    unset (example_SRC)
    aux_source_directory(examples/${EXAMPLE_TARGET} example_SRC)
    add_executable("example_${EXAMPLE_TARGET}" ${example_SRC})
    target_link_libraries("example_${EXAMPLE_TARGET}" awsfx ${Iconv_LIBRARY} ${LIBXML2_LIBRARIES} ${OPENSSL_LIBRARIES} ${CURL_LIBRARIES})
endmacro(add_example_target)

add_example_target(basic)
add_example_target(cxx)


# The Install targets
file (GLOB TARGET_INCS ${CMAKE_SOURCE_DIR}/include/*.h)
install (FILES ${TARGET_INCS} DESTINATION include)

set (CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR})
install (TARGETS ${TARGET} DESTINATION lib)

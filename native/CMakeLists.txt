cmake_minimum_required(VERSION 3.16)
project(qthsharp VERSION 0.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Handy for anyone adding Q_OBJECT classes to native/src later; costs nothing now.
set(CMAKE_AUTOMOC ON)

option(QTH_BUILD_TESTS "Build the native smoke test (needs no display: uses the offscreen QPA)" ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Core: pure C++ classes — handle table, event queue, app host, main window.
set(QTH_CORE_SOURCES
    src/core/handles.cpp
    src/core/events.cpp
    src/core/app_host.cpp
    src/core/hwindow.cpp
    src/core/input_filter.cpp
    src/core/canvas.cpp
    src/core/tree_items.cpp
)

# API: the extern "C" entry points H# links against, grouped by area.
set(QTH_API_SOURCES
    src/api/app_api.cpp
    src/api/widget_api.cpp
    src/api/input_api.cpp
    src/api/layout_api.cpp
    src/api/window_api.cpp
    src/api/dialog_api.cpp
    src/api/misc_api.cpp
    src/api/gfx_api.cpp
    src/api/table_api.cpp
    src/api/tree_api.cpp
    src/api/extra_api.cpp
    src/api/event_api.cpp
)

add_library(qthsharp STATIC ${QTH_CORE_SOURCES} ${QTH_API_SOURCES})

target_include_directories(qthsharp
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(qthsharp PUBLIC Qt6::Widgets)

target_compile_options(qthsharp PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wall -Wextra -Wno-unused-parameter>)

# `extern static [c++, "qthsharp"]` (src/qt_ffi.h#) makes H# link `-lqthsharp`
# and bit exports native/ on LIBRARY_PATH (Bit.hk: native-lib-path), so the
# archive is dropped right here instead of staying in the build tree.
set_target_properties(qthsharp PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

if(QTH_BUILD_TESTS)
    enable_testing()
    add_executable(qth_native_smoke tests/native_smoke.cpp)
    target_link_libraries(qth_native_smoke PRIVATE qthsharp Qt6::Widgets)
    set_target_properties(qth_native_smoke PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    add_test(NAME native_smoke COMMAND qth_native_smoke)
    set_tests_properties(native_smoke PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif()

install(TARGETS qthsharp ARCHIVE DESTINATION lib)
install(FILES include/qthsharp.h DESTINATION include)

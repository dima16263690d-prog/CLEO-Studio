cmake_minimum_required(VERSION 3.24)
project(CLEOStudio VERSION 0.1.0 DESCRIPTION "Open-source script development environment for GTA San Andreas and CLEO" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(CTest)

add_library(cleo_core core/src/lexer.cpp)
target_include_directories(cleo_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/core/include")
target_compile_features(cleo_core PUBLIC cxx_std_20)
if(MSVC)
  target_compile_options(cleo_core PRIVATE /W4 /permissive- /EHsc)
else()
  target_compile_options(cleo_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(cleo-studio-cli app/cli_main.cpp)
target_link_libraries(cleo-studio-cli PRIVATE cleo_core)
find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets)
add_executable(cleo-studio WIN32
  app/main.cpp app/main_window.cpp app/main_window.hpp
  app/script_editor.cpp app/script_editor.hpp
  app/script_highlighter.cpp app/script_highlighter.hpp)
set_target_properties(cleo-studio PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)
target_link_libraries(cleo-studio PRIVATE cleo_core Qt6::Widgets)

if(BUILD_TESTING)
  add_executable(cleo-core-tests tests/lexer_tests.cpp)
  target_link_libraries(cleo-core-tests PRIVATE cleo_core)
  add_test(NAME cleo-core-tests COMMAND cleo-core-tests)
endif()
install(TARGETS cleo-studio cleo-studio-cli RUNTIME DESTINATION bin)

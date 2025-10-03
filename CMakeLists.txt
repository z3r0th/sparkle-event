cmake_minimum_required(VERSION 4.0)
project(SparkleEvents VERSION 0.9.0 LANGUAGES CXX)

include(CMakePackageConfigHelpers)

set(CMAKE_CXX_STANDARD 20)

option(SPARKLE_BUILD_TESTS "Build SparkleEvents test cases" ON)
option(SPARKLE_BUILD_EXAMPLES "Build SparkleEvents examples" ON)

add_library(SparkleEvents INTERFACE
        tests/test_event.cpp)
target_include_directories(SparkleEvents INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(SparkleEvents INTERFACE cxx_std_17)
install(TARGETS SparkleEvents EXPORT SparkleEventsTargets)
install(DIRECTORY include/ DESTINATION include)
install(EXPORT SparkleEventsTargets
        FILE SparkleEventsTargets.cmake
        NAMESPACE Sparkle::
        DESTINATION lib/cmake/SparkleEvents
)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/SparkleEventsConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/SparkleEventsConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/SparkleEventsConfig.cmake"
        INSTALL_DESTINATION lib/cmake/SparkleEvents
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/SparkleEventsConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/SparkleEventsConfigVersion.cmake"
        DESTINATION lib/cmake/SparkleEvents
)

if(SPARKLE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(SPARKLE_BUILD_EXAMPLES)
    #add_subdirectory(examples)
endif()
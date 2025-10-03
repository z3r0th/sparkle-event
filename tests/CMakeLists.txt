# tests/CMakeLists.txt
include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.4 # or latest stable
)
FetchContent_MakeAvailable(Catch2)

add_executable(test_event test_event.cpp)
target_link_libraries(test_event PRIVATE Catch2::Catch2WithMain SparkleEvents)

include(CTest)
include(Catch)
catch_discover_tests(test_event)
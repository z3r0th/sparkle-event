set(CMAKE_CXX_STANDARD 20)
message(STATUS "Building Sparkle Events examples...")

# Define example sources
file(GLOB EXAMPLE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

message(STATUS "Examples: " ${EXAMPLE_SOURCES})

foreach(example_src ${EXAMPLE_SOURCES})
    get_filename_component(example_name ${example_src} NAME_WE)

    add_executable(${example_name} ${example_src})
    target_link_libraries(${example_name} PRIVATE Sparkle::SparkleEvents)

    target_include_directories(${example_name} PRIVATE
            ${CMAKE_SOURCE_DIR}/include
    )

    set_target_properties(${example_name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/examples"
    )

    message(STATUS "Example added: ${example_name}")
endforeach()
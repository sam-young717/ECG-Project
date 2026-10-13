cmake_minimum_required(VERSION 3.15)

set(CPPTEST_HOME /mnt/c/Users/syoung/Documents/CT/cpptest-ct/)

project(CardioMonitor
    VERSION 1.4.2
    DESCRIPTION "Patient Cardiac Vital Signs Monitor Firmware"
    LANGUAGES C CXX)

# Export compile_commands.json for static analysis tools (e.g. Parasoft C/C++test).
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(CPPTEST_COVERAGE)
  include(/mnt/c/Users/syoung/Documents/CT/cpptest-ct/integration/cmake/cpptest-coverage.cmake)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Warning flags representative of an embedded firmware build.
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wshadow
        -Wpointer-arith
        -Wcast-align
        -Wstrict-prototypes
        -Wmissing-prototypes
        -fno-common
    )
endif()

# Simulated target defines. On the real MCU these come from the BSP.
add_compile_definitions(
    CARDIO_MONITOR_FW_VERSION="${PROJECT_VERSION}"
    ECG_SAMPLE_RATE_HZ=250
    SPO2_SAMPLE_RATE_HZ=100
    SYSTEM_TICK_MS=10
)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(CARDIO_SOURCES
    src/main.c
    src/ecg_processor.c
    src/spo2_monitor.c
    src/nibp_monitor.c
    src/alarm_manager.c
    src/patient_record.c
    src/data_logger.c
    src/comm_link.c
    src/vital_telemetry.c
)

add_executable(cardio_monitor ${CARDIO_SOURCES})

target_link_libraries(cardio_monitor PRIVATE m)

install(TARGETS cardio_monitor RUNTIME DESTINATION bin)

# Keep third-party and test targets out of the production compile database.
set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
)
FetchContent_MakeAvailable(googletest)

add_executable(cardio_tests
    tests/ecg_processor_test.cpp
    tests/spo2_monitor_test.cpp
    tests/nibp_monitor_test.cpp
    tests/alarm_manager_test.cpp
    tests/patient_record_test.cpp
    tests/data_logger_test.cpp
    tests/comm_link_test.cpp
    src/ecg_processor.c
    src/spo2_monitor.c
    src/nibp_monitor.c
    src/alarm_manager.c
    src/patient_record.c
    src/data_logger.c
    src/comm_link.c
)
set_target_properties(cardio_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build"
)
target_include_directories(cardio_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_compile_features(cardio_tests PRIVATE cxx_std_17)
target_link_libraries(cardio_tests PRIVATE GTest::gtest_main m)

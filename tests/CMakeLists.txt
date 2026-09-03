add_executable(popnet_unit_tests
    unit_tests.cpp
)
target_link_libraries(popnet_unit_tests PRIVATE Popnet::Core)
target_compile_definitions(popnet_unit_tests PRIVATE
    POPNET_TEST_FIXTURES="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)
target_compile_options(popnet_unit_tests PRIVATE
    -Wall -Wextra -Wpedantic -Wconversion -Wshadow
)

add_executable(popnet_integration_tests
    integration_tests.cpp
)
target_link_libraries(popnet_integration_tests PRIVATE Popnet::Core)
target_compile_definitions(popnet_integration_tests PRIVATE
    POPNET_TEST_FIXTURES="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)
target_compile_options(popnet_integration_tests PRIVATE
    -Wall -Wextra -Wpedantic -Wconversion -Wshadow
)

add_test(NAME popnet.unit COMMAND popnet_unit_tests)
add_test(NAME popnet.integration COMMAND popnet_integration_tests)

set_tests_properties(popnet.unit popnet.integration PROPERTIES
    TIMEOUT 60
)

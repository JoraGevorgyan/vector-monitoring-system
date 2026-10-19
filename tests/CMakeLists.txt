file(GLOB_RECURSE UNIT_TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/tests/unit/*.cpp)
file(GLOB_RECURSE INTEGRATION_TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/tests/integration/*.cpp)

find_package(GTest CONFIG REQUIRED)

if(UNIT_TEST_SOURCES)
	add_executable(unit_tests ${UNIT_TEST_SOURCES})
	target_link_libraries(unit_tests PRIVATE vm_lib GTest::gtest_main)
	target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)
	add_test(NAME unit_tests COMMAND unit_tests)
endif()

if(INTEGRATION_TEST_SOURCES)
	add_executable(integration_tests ${INTEGRATION_TEST_SOURCES})
	target_link_libraries(integration_tests PRIVATE vm_lib GTest::gtest_main)
	target_include_directories(integration_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)
	add_test(NAME integration_tests COMMAND integration_tests)
endif()

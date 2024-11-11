file(GLOB SAMPLES_SOURCE "*.cpp")

add_custom_target(BUILD_TESTS)

foreach (sample_source ${SAMPLES_SOURCE})
    get_filename_component(sample_name ${sample_source} NAME_WE)
    add_executable(${sample_name} ${sample_source})
    add_dependencies(BUILD_TESTS ${sample_name})
endforeach ()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/TestData DESTINATION ${CMAKE_BINARY_DIR}/Tests)

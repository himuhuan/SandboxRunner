set(SANITIZER_SAMPLE_OUTPUT_DIR ${CMAKE_BINARY_DIR}/Tests/SanitizerSamples)
set(SANITIZER_SAMPLE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/../Samples/ExpectedAccepted.cpp)

if (MSVC)
    add_custom_target(
            SANITIZER_SAMPLES_TESTS
            COMMAND ${CMAKE_COMMAND} -E make_directory ${SANITIZER_SAMPLE_OUTPUT_DIR}
            COMMAND ${CMAKE_COMMAND} -E echo "Skipping sanitizer sample generation on MSVC"
            COMMENT "Generating sanitizer instrumented samples..."
    )
else ()
    add_custom_target(
            SANITIZER_SAMPLES_TESTS
            COMMAND ${CMAKE_COMMAND} -E make_directory ${SANITIZER_SAMPLE_OUTPUT_DIR}
            COMMAND ${CMAKE_CXX_COMPILER} -std=c++20 -O2 -fsanitize=undefined
                    ${SANITIZER_SAMPLE_SOURCE}
                    -o ${SANITIZER_SAMPLE_OUTPUT_DIR}/ExpectedAcceptedUbsan
            COMMAND ${CMAKE_CXX_COMPILER} -std=c++20 -O2 -fsanitize=address,undefined
                    ${SANITIZER_SAMPLE_SOURCE}
                    -o ${SANITIZER_SAMPLE_OUTPUT_DIR}/ExpectedAcceptedAsanUbsan
            COMMENT "Generating sanitizer instrumented samples..."
    )
endif ()

add_dependencies(BUILD_ALL SANITIZER_SAMPLES_TESTS)

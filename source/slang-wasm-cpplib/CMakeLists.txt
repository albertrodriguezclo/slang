#
# WebAssembly C++ library for Slang
#
if(EMSCRIPTEN)
    slang_add_target(
        .
        STATIC
        EXCLUDE_FROM_ALL
        USE_FEWER_WARNINGS
        LINK_WITH_PRIVATE
            miniz
            lz4_static
            slang
            core
            compiler-core
            slang-capability-defs
            slang-capability-lookup
            slang-fiddle-output
            slang-lookup-tables
        INCLUDE_DIRECTORIES_PUBLIC ${slang_SOURCE_DIR}/include .
    )
    # To be emscripten compilation friendly
    target_link_options(
            slang-wasm-cpplib
            PUBLIC
            "-s" "WASM=1"
#           "-s" "SIDE_MODULE=1"
            "-s" "LINKABLE=1"
            "-s" "EXPORT_ALL=1"
    )
    # Install the library and header for distribution
    install(TARGETS slang-wasm-cpplib
            ARCHIVE DESTINATION lib
            COMPONENT slang-wasm-cpplib)

    install(FILES slang-wasm-cpplib.h
            DESTINATION include
            COMPONENT slang-wasm-cpplib)

endif()


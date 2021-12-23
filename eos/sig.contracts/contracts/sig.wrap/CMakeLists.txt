add_contract(sig.wrap sig.wrap ${CMAKE_CURRENT_SOURCE_DIR}/src/sig.wrap.cpp)

target_include_directories(sig.wrap
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(sig.wrap
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/sig.wrap.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/sig.wrap.contracts.md @ONLY )

target_compile_options( sig.wrap PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )

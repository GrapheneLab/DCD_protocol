add_contract(sig.token sig.token ${CMAKE_CURRENT_SOURCE_DIR}/src/sig.token.cpp)

target_include_directories(sig.token
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(sig.token
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/sig.token.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/sig.token.contracts.md @ONLY )

target_compile_options( sig.token PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )

add_contract(sig.msig sig.msig ${CMAKE_CURRENT_SOURCE_DIR}/src/sig.msig.cpp)

target_include_directories(sig.msig
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(sig.msig
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/sig.msig.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/sig.msig.contracts.md @ONLY )

target_compile_options( sig.msig PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )

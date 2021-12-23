add_contract(sig.bios sig.bios ${CMAKE_CURRENT_SOURCE_DIR}/src/sig.bios.cpp)

target_include_directories(sig.bios
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(sig.bios
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/sig.bios.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/sig.bios.contracts.md @ONLY )

target_compile_options( sig.bios PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )

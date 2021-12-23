file(GLOB HEADERS "include/eosio/gate_plugin/*.hpp")
add_library( gate_plugin
             gate_plugin.cpp
             ${HEADERS} )

target_link_libraries( gate_plugin eosio_chain chain_plugin appbase fc )
target_include_directories( gate_plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )



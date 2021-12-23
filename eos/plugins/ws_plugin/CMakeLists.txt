file(GLOB HEADERS "include/eosio/ws_plugin/*.hpp")
add_library( ws_plugin
             ws_plugin.cpp
             ${HEADERS} )

target_link_libraries( ws_plugin eosio_chain chain_plugin appbase fc )
target_include_directories( ws_plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )



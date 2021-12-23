# will be implictly used for any compilation unit if not overrided by SYSTEM_INCLUDE_FOLDERS parameter
# these directories go as -isystem <dir> to avoid warnings from code of third-party libraries

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/sig.bios/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/sig.bios/)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/sig.msig/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/sig.msig/)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/sig.system/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/sig.system/)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/sig.token/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/sig.token/)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/sig.wrap/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/sig.wrap/)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/old_versions/v1.6.0-rc3/sig.bios/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/old_versions/v1.6.0-rc3/sig.bios/)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/old_versions/v1.7.0-develop-preactivate_feature/sig.bios/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/old_versions/v1.7.0-develop-preactivate_feature/sig.bios/)

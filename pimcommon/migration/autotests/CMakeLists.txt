include_directories(${CMAKE_SOURCE_DIR}/pimcommon/ ${CMAKE_BINARY_DIR}/pimcommon/
)

add_definitions( -DMIGRATION_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data" )

set( pimcommon_migrateapplicationfilestest_source migrateapplicationfilestest.cpp ../migrateapplicationfiles.cpp ../migratefileinfo.cpp)
add_executable(migrateapplicationfilestest ${pimcommon_migrateapplicationfilestest_source})
ecm_mark_as_test(pimcommon-migrateapplicationfilestest)
target_link_libraries( migrateapplicationfilestest Qt5::Test KF5::CoreAddons KF5::ConfigCore)



set( pimcommon_migratefileinfotest_source migratefileinfotest.cpp ../migratefileinfo.cpp)
add_executable(migratefileinfotest ${pimcommon_migratefileinfotest_source})
ecm_mark_as_test(pimcommon-migratefileinfotest)
target_link_libraries( migratefileinfotest Qt5::Test )


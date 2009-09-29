## This file should be placed in the root directory of your project.
## Then modify the CMakeLists.txt file in the root directory of your
## project to incorporate the testing dashboard.
## # The following are required to uses Dart and the Cdash dashboard
##   ENABLE_TESTING()
##   INCLUDE(CTest)
set(CTEST_PROJECT_NAME "kdepimlibs")
set(CTEST_NIGHTLY_START_TIME "00:00:00 UTC")

set(CTEST_DROP_METHOD "http")
set(CTEST_DROP_SITE "dashboard.akonadi-project.org")
set(CTEST_DROP_LOCATION "/CDash/submit.php?project=kdepim")
set(CTEST_DROP_SITE_CDASH TRUE)

set(CTEST_CUSTOM_MAXIMUM_NUMBER_OF_WARNINGS 1000)

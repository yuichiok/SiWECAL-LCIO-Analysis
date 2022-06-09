# project name
PROJECT( lcio_bcid_correlations )


# project version
cmake_minimum_required(VERSION 3.10)

# install binary here:
SET(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}" CACHE PATH "Where to install ${PROJECT_NAME}" FORCE)

SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

find_package(LCIO)
find_package(ROOT)

# build the executable and put it into bin/
add_executable(lcio_bcid_correlations lcio_bcid_correlations.cc)
#target_include_directories(lcio_bcid_correlations ${LCIO_INCLUDE_DIRS})
target_link_libraries(lcio_bcid_correlations ${LCIO_LIBRARIES} ${ROOT_LIBRARIES} ROOT::Core ROOT::RIO ROOT::Tree ROOT::Physics  ROOT::GenVector ROOT::MathCore)
install(TARGETS lcio_bcid_correlations DESTINATION bin)

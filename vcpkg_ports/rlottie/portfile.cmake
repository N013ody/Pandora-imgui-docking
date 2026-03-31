vcpkg_from_git(
    OUT_SOURCE_PATH SOURCE_PATH
    URL https://github.com/Samsung/rlottie.git
    REF 671c561130ead1c6e44805a7ec1263573a3440fd
    HEAD_REF master
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DLOTTIE_MODULE=OFF
        -DLOTTIE_THREAD=OFF
        -DBUILD_SHARED_LIBS=OFF
)

vcpkg_cmake_install()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

# Handle copyright
if(EXISTS "${SOURCE_PATH}/COPYING")
    file(INSTALL "${SOURCE_PATH}/COPYING" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
elseif(EXISTS "${SOURCE_PATH}/LICENSE")
    file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
else()
    file(WRITE "${CURRENT_PACKAGES_DIR}/share/${PORT}/copyright" "rlottie is licensed under the MIT and BSD-3-Clause licenses. See https://github.com/Samsung/rlottie for details.")
endif()

# Create a simple CMake config file for find_package support
file(WRITE "${CURRENT_PACKAGES_DIR}/share/${PORT}/rlottie-config.cmake" [[
@PACKAGE_INIT@

include(CMakeFindDependencyMacro)

if(NOT TARGET rlottie::rlottie)
    add_library(rlottie::rlottie STATIC IMPORTED)
    set_target_properties(rlottie::rlottie PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${PACKAGE_PREFIX_DIR}/include"
        IMPORTED_LOCATION "${PACKAGE_PREFIX_DIR}/lib/rlottie.lib"
        IMPORTED_LOCATION_DEBUG "${PACKAGE_PREFIX_DIR}/debug/lib/rlottie.lib"
    )
endif()

check_required_components(rlottie)
]])

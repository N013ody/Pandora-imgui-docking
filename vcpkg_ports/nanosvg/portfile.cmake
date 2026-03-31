vcpkg_from_git(
    OUT_SOURCE_PATH SOURCE_PATH
    URL https://github.com/memononen/nanosvg.git
    REF 5cefd9847949af6df13f65027fd43af5a7513633
    HEAD_REF master
)

# nanosvg is a header-only library
# Just install the headers
file(INSTALL "${SOURCE_PATH}/src/nanosvg.h" DESTINATION "${CURRENT_PACKAGES_DIR}/include")
file(INSTALL "${SOURCE_PATH}/src/nanosvgrast.h" DESTINATION "${CURRENT_PACKAGES_DIR}/include")

# Create a CMake config file for find_package support
file(WRITE "${CURRENT_PACKAGES_DIR}/share/${PORT}/nanosvg-config.cmake" [[
@PACKAGE_INIT@

include(CMakeFindDependencyMacro)

# nanosvg is header-only, just set the include directory
set_and_check(nanosvg_INCLUDE_DIR "${PACKAGE_PREFIX_DIR}/include")

if(NOT TARGET nanosvg::nanosvg)
    add_library(nanosvg::nanosvg INTERFACE IMPORTED)
    set_target_properties(nanosvg::nanosvg PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${nanosvg_INCLUDE_DIR}"
    )
endif()

if(NOT TARGET nanosvg::nanosvgrast)
    add_library(nanosvg::nanosvgrast INTERFACE IMPORTED)
    set_target_properties(nanosvg::nanosvgrast PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${nanosvg_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES nanosvg::nanosvg
    )
endif()

check_required_components(nanosvg)
]])

# Handle copyright
if(EXISTS "${SOURCE_PATH}/LICENSE.txt")
    file(INSTALL "${SOURCE_PATH}/LICENSE.txt" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
elseif(EXISTS "${SOURCE_PATH}/LICENSE")
    file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
else()
    file(WRITE "${CURRENT_PACKAGES_DIR}/share/${PORT}/copyright" "NanoSVG is licensed under the zlib license. See https://github.com/memononen/nanosvg for details.")
endif()

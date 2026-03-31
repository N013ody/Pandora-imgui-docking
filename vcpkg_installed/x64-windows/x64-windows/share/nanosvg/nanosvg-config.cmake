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

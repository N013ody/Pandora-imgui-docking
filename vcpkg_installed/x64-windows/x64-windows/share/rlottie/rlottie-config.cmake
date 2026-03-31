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

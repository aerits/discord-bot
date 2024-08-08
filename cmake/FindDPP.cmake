find_path(DPP_INCLUDE_DIR NAMES dpp/dpp.h HINTS ${DPP_ROOT_DIR} /usr/local/include /usr/include)

find_library(DPP_LIBRARIES NAMES dpp "libdpp.a" HINTS ${DPP_ROOT_DIR} /usr/local/lib /usr/lib)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(DPP DEFAULT_MSG DPP_LIBRARIES DPP_INCLUDE_DIR)

if (DPP_FOUND)
	set(DPP_LIBRARIES ${DPP_LIBRARIES})
	set(DPP_INCLUDE_DIRS ${DPP_INCLUDE_DIRS})
endif()

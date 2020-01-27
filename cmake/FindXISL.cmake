set(XISL_INCLUDE_DIRS)
set(XISL_LIBRARIES)
set(XISL_DEFINITIONS)

if(WIN32)
  # Missing find_path here
  find_library(XISL_LIBRARIES XISL)
  find_path(XISL_INCLUDE_DIRS "Acq.h")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(XISL DEFAULT_MSG
  XISL_LIBRARIES
  XISL_INCLUDE_DIRS
)

#find_package(GTest CONFIG REQUIRED) # link GTest::gtest_main with this

#To make usable in debug/release/etc. configurations
include(FetchContent)
FetchContent_Declare(  googletest
  # Specify the commit you depend on and update it regularly.
  URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
  )
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF)
set(INSTALL_GTEST OFF)
FetchContent_MakeAvailable(googletest)


function(create_test name sources)
  add_executable(${name} ${${sources}})
  target_link_libraries(${name} gtest_main lib)
  add_test(${name} ${name} COMMAND ${name})
  set_target_properties(${name} PROPERTIES FOLDER "Tests")
endfunction()

set(test_files distance_of_segments.cpp)
create_test(distance_of_segments test_files)

set(test_files aabb_test.cpp)
create_test(aabb_test test_files)

#set(test_files runtime_complexity.cpp)
#create_test(runtime_complexity test_files)

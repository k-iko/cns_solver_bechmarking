cmake_minimum_required(VERSION 3.9) # 3.9 To rely on modern OpenMP support
project(main_tw)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
find_package(OpenMP)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
if(${MPI_FOUND})
  set(CMAKE_CXX_COMPILE_FLAGS ${CMAKE_CXX_COMPILE_FLAGS} ${MPI_COMPILE_FLAGS})
  set(CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} ${MPI_LINK_FLAGS})
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMPI_ENABLED")
  include_directories(${MPI_INCLUDE_PATH})
endif()
add_subdirectory(pybind11)
add_library(common ../Vehicle.cpp
				   ../Common/compare_time.cpp 
				   ../Common/return_lutime_penalty.cpp
				   ../Common/get_dtable.cpp
				   ../Common/get_ttable.cpp
				   ../Common/get_etable.cpp
				   ../Common/get_xtable.cpp
				   ../Common/calc_dist.cpp
				   ../Common/calc_power.cpp
				   ../Common/calc_exp.cpp
				   ../Common/calc_time_pen.cpp
				   ../Common/max_dist_from_table.cpp
				   ../Common/max_e_from_table.cpp
				   ../Common/check_impassability.cpp
				   ../Common/improved_or_not.cpp
				   ../Common/backup_route.cpp
				   ../Common/generate_routefile.cpp
				   ../Common/generate_resultfile.cpp
				   ../Common/outputs_middle.cpp
				   ../Common/calc_avetime_APE.cpp
				   ../Common/calc_avetime_AE.cpp
				   ../Common/calc_avevisits_APE.cpp
				   ../Common/calc_avevisits_AE.cpp
				   ../Common/calc_vehicle_loadpickup_penalty.cpp
				   ../Common/find_power_to_charge.cpp
				   ../Common/update_remdist.cpp
				   ../Common/update_load_onbrd.cpp
				   ../Common/update_arrtime_tw.cpp
				   ../Common/update_rem_battery.cpp
				   ../Common/update_totalcost_dtable.cpp
				   ../Common/delay_arr_time.cpp
				   ../Common/delay_arr_time2.cpp)
add_library(cross ../Cross/cross_cust_dtable.cpp
				  ../Cross/cross_improved_lastc_dtable.cpp
				  ../Cross/cross_improved_todepot_dtable.cpp)
pybind11_add_module(cross_dtable ../cross_dtable.cpp)
target_link_libraries(cross_dtable PRIVATE common cross)
target_link_libraries(cross_dtable PUBLIC ${MPI_C_LIBRARIES})
if(OpenMP_CXX_FOUND)
  target_link_libraries(cross_dtable PUBLIC OpenMP::OpenMP_CXX)
else()
  message( FATAL_ERROR "Your compiler does not support OpenMP" )
endif()


#find_package(raylib CONFIG REQUIRED)
#target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

#find_package(glfw3 CONFIG REQUIRED)
#	target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

#find_package(EnTT CONFIG REQUIRED)
#    target_link_libraries(${PROJECT_NAME} PRIVATE EnTT::EnTT)
#
#find_package(nlohmann_json CONFIG REQUIRED)
#    target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

#find_package(glm CONFIG REQUIRED)
#	target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)

find_package(zstd CONFIG REQUIRED)

if(COMPILE_EXECUTABLE)
	target_link_libraries(${ProjectTest} PRIVATE zstd)
endif()

target_link_libraries(${ProjectLibrary} PRIVATE zstd)

set (PROJECTS
    starfield
    menger_sponge_fractal
    snake
    purple_rain
    space_invader
    mitosis
    solar
    maze_generator
    noise_terrain
    lorenz_system
    reaction_diffusion
    fractal_tree
    lsystem_tree
    space_colonization_tree_3d
    firework
    poisson_disk_sample
    levy_flight
    attraction_repulsion
    steer
    fractal_spirograph
    cube_wave
    knot
    snow
    double_pendulum
    )

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}"  )

foreach(PROJECT ${DEMOES})
    message("add challenge ${PROJECT}")

    add_executable(${PROJECT} ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT}/${PROJECT}.cpp)
    set_target_properties(${PROJECT} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
    target_link_libraries(${PROJECT}
        PRIVATE
            common
            osg3::osgViewer
            osg3::osgAnimation
        )

    # copy project_data to target file dir.
    set(${PROJECT}_data ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT}/${PROJECT}_data)
    IF(EXISTS ${${PROJECT}_data})
        # *_BUILD custom_command only executed when target get build, but i
        # want data copied no matter target get build or not
        #
        # add_custom_command(TARGET ${PROJECT} POST_BUILD COMMAND
        #     ${CMAKE_COMMAND} -E copy_directory ${${PROJECT}_data}
        #     $<TARGET_FILE_DIR:${PROJECT}>/${PROJECT}_data)

        # you can not use $<TARGET_FIL_DIR:...> here, it cause cycle dependencies error
        add_custom_target(${PROJECT}_data COMMAND
              ${CMAKE_COMMAND} -E copy_directory ${${PROJECT}_data}
              ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT}_data)
        add_dependencies(${PROJECT} ${PROJECT}_data)
        message("depend ${PROJECT} on ${PROJECT}_data")
    ENDIF()
endforeach()

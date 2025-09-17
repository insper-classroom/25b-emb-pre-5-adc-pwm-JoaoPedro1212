add_executable(exe4
        main.c
)

target_link_libraries(exe4
                     hardware_adc
                     hardware_timer
                     pico_stdlib )
pico_add_extra_outputs(exe4)

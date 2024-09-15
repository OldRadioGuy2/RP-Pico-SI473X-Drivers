add_library(si4735 SI4735.cpp Wire.cpp Serial.cpp )
#target_include_directories(si4735 PUBLIC ${PICO_INCLUDE_PATH})
target_link_libraries(si4735  pico_stdlib hardware_i2c hardware_pwm hardware_clocks)


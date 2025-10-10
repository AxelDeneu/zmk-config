# Custom OLED status screen
if(CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM)
    target_sources(app PRIVATE sofle_status_screen.c)
endif()

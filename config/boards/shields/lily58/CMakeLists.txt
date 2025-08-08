if(CONFIG_ZMK_WIDGET_BONGO_CAT)
  target_sources(app PRIVATE widgets/bongo_cat.c)
  target_include_directories(app PRIVATE widgets)
endif()
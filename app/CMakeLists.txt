file(GLOB_RECURSE SQUARELINE_SRCS gui/squareline_ui/*.c)
file(GLOB GUI_SRCS gui/*.c)
idf_component_register(SRCS "app_fsm.c" "app_pipeline.c" "app_gui.c" ${SQUARELINE_SRCS} ${GUI_SRCS}
                    INCLUDE_DIRS "." "gui" "gui/squareline_ui"
                    REQUIRES bsp os svc lvgl esp_lvgl_port)

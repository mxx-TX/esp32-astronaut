idf_component_register(SRCS "os_task.c" "os_queue.c" "os_mutex.c" "os_event.c" "os_mem.c" "test_os.c"
                    INCLUDE_DIRS "."
                    REQUIRES freertos esp_hw_support heap)

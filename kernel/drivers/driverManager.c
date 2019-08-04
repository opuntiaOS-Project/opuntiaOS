#include <driverManager.h>
#include <keyboard.h>

void register_drivers() {
    drivers_cnt = 0;
    register_keyboard_driver();
}

uint8_t register_driver(bool *activate_function(void), void *stop_function(void)) {
    driver_t new_driver;
    new_driver.activate = activate_function;
    new_driver.stop = stop_function;
    drivers[drivers_cnt++] = new_driver;
}

bool start_driver(uint8_t id) {
    bool (*func)(void) = drivers[id].activate;
    return func();
}

void stop_driver(uint8_t id) {
    void (*func)(void) = drivers[id].stop;
    func();
}

bool start_all_drivers() {
    bool all_started_successfully = 1;
    for (uint8_t driver_id = 0; driver_id < drivers_cnt; driver_id++) {
        all_started_successfully |= start_driver(driver_id);
    }
    return all_started_successfully;
}

void stop_all_drivers() {
    for (uint8_t driver_id = 0; driver_id < drivers_cnt; driver_id++) {
        stop_driver(driver_id);
    }
}
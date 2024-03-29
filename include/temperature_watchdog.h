#ifndef TEMPERATURE_WATCHDOG_H__
#define TEMPERATURE_WATCHDOG_H__


#define NUMBER_OF_OUTPUTS 2
#define TEMPERATURE_WATCHDOG_INTERVAL_MS 30000
#define TEMPERATURE_WATCHDOG_TEMPERATURE_OUTDATED_MS 45000
#define TEMPERATURE_WATCHDOG_MAX_TEMP 80

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#include <zephyr/types.h>

#ifdef __cplusplus
extern "C" {
#endif

enum output_command_type {
    OUTPUT_COMMAND_TYPE_ONOFF,
    OUTPUT_COMMAND_TYPE_LEVEL,
};

struct output_command {
    int cmd_type;
    union {
        bool off_value;
        uint16_t off_level;
    };
    //these functions shall return what they have physically set on the output
    union {
        bool (*gpio_set)(bool value);
        uint16_t (*pwm_set)(uint16_t value);
    };
    // these function pointers shall update 
    // the respective structs of the appliances
    // e.g. when all appliances had to be turned off
    union {
        void (*gpio_update)(bool current_value);
        void (*pwm_update)(uint16_t current_level);
    };
};

struct temp_watchdog_ctx {
    //last read temperature
    int16_t temp;
    //time when temperature was last read
    int64_t last_fetched;
    //function that reads & returns the current temperature
    int16_t (*fetch_temperature)(void);
    //work function that gets scheduled regularly
    struct k_work_delayable work;
    //array of all output commands that shall be affected when overheating occurs
    struct output_command* output_commands[NUMBER_OF_OUTPUTS];
    //functions to execute when overheating occurred ...
    void (*notify_overheating)(void);
    // .. and pcb cooled down again
    void (*notify_normalized)(void);
    // store if currently overheated
    bool overheated;
};


/// @brief initialize the temperature watchdog
/// @param temp_ctx helper struct
/// @param fetch_temp function that returns the current temperature
/// @return 0 on success
int init_temperature_watchdog(struct temp_watchdog_ctx *temp_ctx, 
            int16_t (*fetch_temp)(void),
            void (*notify_overheat)(void), 
            void (*notify_ok)(void));


/// @brief wrapper function that only switches appliance on or off 
/// if temperature is within bounds
/// @param temp_ctx helper struct
/// @param cmd_number number that was assigned when command was registered
/// (normally in form of an enum)
/// @param onOFF_value the value to be set
/// @return onOff_value that has been physically set
bool safely_switch_onOff(struct temp_watchdog_ctx *temp_ctx, 
            const int cmd_number, 
            const bool onOFF_value);


/// @brief wrapper function that only switches appliance to new level
/// if temperature is within bounds
/// @param temp_ctx helper struct
/// @param cmd_number number that was assigned when command was registered
/// (normally in form of an enum)
/// @param level level to be set
/// @return level that has been physically set
uint16_t safely_switch_level(struct temp_watchdog_ctx *temp_ctx, 
            const int cmd_number, 
            const uint16_t level);


/// @brief register an output command that shall be affected by the watchdog
/// @param temp_ctx helper struct
/// @param cmd_ctx predefined struct of the command 
/// (including type, off value, set function)
/// @param cmd_number number that shall be assigned to the command
/// (normally in form of an enum)
/// @return 0 on success
int register_output_cmd(struct temp_watchdog_ctx *temp_ctx, 
            struct output_command *cmd_ctx, 
            const int cmd_number);


/// @brief remove an output command to not be longer affected by the watchdog
/// @param temp_ctx helper struct
/// @param cmd_number number that was assigned when command was registered
/// (normally in form of an enum)
/// @return 0 on success
int deregister_output_cmd(struct temp_watchdog_ctx *temp_ctx,
            const int cmd_number);






#ifdef __cplusplus
}
#endif

#endif /* TEMPERATURE_WATCHDOG_H__ */
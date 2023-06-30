#include "temperature_watchdog.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(heat_watchdog,LOG_LEVEL_DBG);

// ==================== helper functions ==================================== //

/// @brief check if command number is within bounds
/// @param cmd_number 
/// @return true if within boundary
static bool check_cmd_number(const int cmd_number)
{
    if(cmd_number < 0 || cmd_number >= NUMBER_OF_OUTPUTS) {
        LOG_ERR("Command number out of bounds");
        return false;
    }
    return true;
}





/// @brief get the output command struct for a given command number
/// @param temp_ctx the temperature watchdog instance
/// @param cmd_number command number
static struct output_command* get_output_cmd(const struct temp_watchdog_ctx *temp_ctx, 
            const int cmd_number)
{
    if(!check_cmd_number(cmd_number)) {
        return NULL;
    }
    return temp_ctx->output_commands[cmd_number];
}





// ==================== private functions ==================================== //

/// @brief check if temperature is within bounds and up to date
/// @param ctx helper struct
/// @return true if temperature is within bounds, false if not
static bool proof_legal_temperature(struct temp_watchdog_ctx *ctx)
{
    int64_t oldness_temp_value = k_uptime_delta(&ctx->last_fetched);
    if(oldness_temp_value > TEMPERATURE_WATCHDOG_TEMPERATURE_OUTDATED_MS) {
        LOG_WRN("Temperature value is too old: %lld ms", oldness_temp_value);
        return false;
    }
    bool temp_ok = (ctx->temp <= TEMPERATURE_WATCHDOG_MAX_TEMP);
    if(!temp_ok) {
        LOG_WRN("Temperature out of bounds: %d", ctx->temp);
        return false;
    }
    return true;
}




/// @brief fetch temperature and the current timestamp
/// @param ctx helper struct to which values shall be saved
static void fetch_temp_and_time(struct temp_watchdog_ctx *ctx)
{
    ctx->temp = ctx->fetch_temperature();
    ctx->last_fetched = k_uptime_get();
    LOG_DBG("Fetched temperature: %d & timestamp %lld", ctx->temp, ctx->last_fetched);
}





/// @brief turn off all appliances to their default value
/// @param temp_ctx helper struct
/// @return number of appliances that were turned off
static int turn_off_all_appliances(const struct temp_watchdog_ctx *temp_ctx)
{
    int success_counter = 0;
    for(size_t i = 0; i < NUMBER_OF_OUTPUTS; i++) {
        struct output_command *cmd_temp = get_output_cmd(temp_ctx, i);
        if(cmd_temp == NULL) {
            LOG_DBG("No command for output %d", i);
            continue;
        }
        switch (cmd_temp->cmd_type)
        {
        case OUTPUT_COMMAND_TYPE_ONOFF:
            cmd_temp->gpio_set(cmd_temp->off_value);
            cmd_temp->gpio_update(cmd_temp->off_value);
            success_counter++;
            LOG_DBG("Turned relais %d to %d", i, cmd_temp->off_value);
            break;
        case OUTPUT_COMMAND_TYPE_LEVEL:
            cmd_temp->pwm_set(cmd_temp->off_level);
            cmd_temp->pwm_update(cmd_temp->off_level);
            success_counter++;
            LOG_DBG("Turned dimmer %d to %d", i, cmd_temp->off_level);
            break;
        default:
            LOG_ERR("Error emergency turn off. Cmd type for cmd %d was not found, appliance could not be turned off!!!", i);
            break;
        }
    }
    return success_counter;
}





/// @brief work function that gets scheduled regularly
/// @param work 
static void temp_watchdog_work(struct k_work *work)
{
    LOG_DBG("Temp watchdog work called");
    struct temp_watchdog_ctx *ctx = 
        CONTAINER_OF(work, struct temp_watchdog_ctx, work.work);
    //fetch temperature
    fetch_temp_and_time(ctx);

    //if currently overheated -> check if temperature is normalized and notify
    // ---
    //if currently not overheated -> check if temperature is too high, notify
    // and turn all appliances of
    bool temperatureOK = proof_legal_temperature(ctx);
    if(true == ctx->overheated){
        if(temperatureOK){
            ctx->notify_normalized();
            ctx->overheated = false;
        } else {
            LOG_DBG("Temperature is still too high");
        }
    } else {    //in past not overheated
        if(!temperatureOK){
            ctx->notify_overheating();
            ctx->overheated = true;
            LOG_INF("All appliances will be turned off");
            turn_off_all_appliances(ctx);
        }
        // everything ok, no need to take action
    }

    //reschedule work
    k_work_reschedule(&ctx->work, K_MSEC(TEMPERATURE_WATCHDOG_INTERVAL_MS));
}





// ==================== public functions ==================================== //

int init_temperature_watchdog(struct temp_watchdog_ctx *temp_ctx, 
            int16_t (*fetch_temp)(void), 
            void (*notify_overheat)(void), 
            void (*notify_ok)(void))
{
    temp_ctx->fetch_temperature = fetch_temp;
    k_work_init_delayable(&temp_ctx->work, temp_watchdog_work);
    k_work_reschedule(&temp_ctx->work, K_NO_WAIT);
    temp_ctx->notify_overheating = notify_overheat;
    temp_ctx->notify_normalized = notify_ok;
    temp_ctx->overheated = false;
    LOG_INF("Initialized temperature watchdog");
    return 0;
}






bool safely_switch_onOff(struct temp_watchdog_ctx *temp_ctx, 
            const int cmd_number, 
            const bool onOFF_value)
{
    struct output_command *cmd_temp = get_output_cmd(temp_ctx, cmd_number);
    if(!proof_legal_temperature(temp_ctx)) {
        LOG_WRN("Can't switch, temperature is out of bounds");
    } else {
        if(cmd_temp->cmd_type == OUTPUT_COMMAND_TYPE_ONOFF) {
            LOG_DBG("Safely switch to %d", onOFF_value);
            return cmd_temp->gpio_set(onOFF_value);
        } else {
            LOG_ERR("Wrong command type");
        }
    }
    return cmd_temp->off_value;
}





uint16_t safely_switch_level(struct temp_watchdog_ctx *temp_ctx, 
            const int cmd_number, 
            const uint16_t level)
{
    struct output_command *cmd_temp = get_output_cmd(temp_ctx, cmd_number);
    if(!proof_legal_temperature(temp_ctx)) {
        LOG_WRN("Can't switch, temperature is out of bounds");
    } else {
        if(cmd_temp->cmd_type == OUTPUT_COMMAND_TYPE_LEVEL) {
            LOG_DBG("Safely set value to %d", level);
            return cmd_temp->pwm_set(level);
        } else {
            LOG_ERR("Wrong command type");
        }
    }
    return cmd_temp->off_level;
}





int register_output_cmd(struct temp_watchdog_ctx *temp_ctx, 
            struct output_command *cmd_ctx, 
            const int cmd_number)
{
    if(!check_cmd_number(cmd_number)) {
        return -1;
    }
    temp_ctx->output_commands[cmd_number] = cmd_ctx;
    LOG_DBG("Registered cmd with type %d under %d", 
        cmd_ctx->cmd_type, cmd_number);
    return 0;
}




int deregister_output_cmd(struct temp_watchdog_ctx *temp_ctx, 
            const int cmd_number)
{
    if(!check_cmd_number(cmd_number)) {
        return -1;
    }
    LOG_DBG("Removed cmd with type %d under %d", 
        temp_ctx->output_commands[cmd_number]->cmd_type, cmd_number);
    temp_ctx->output_commands[cmd_number] = NULL;
    return 0;
}

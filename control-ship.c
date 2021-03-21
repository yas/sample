#include <errno.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "i2c_rw.h"

typedef struct {
    uint8_t address_;
    uint8_t value_[1];
} Command;

typedef struct {
    const char *target_;
    uint8_t address_;
    long parameter_min_;
    long parameter_max_;
    uint8_t (*calc_function_)(const long param);
} Argument;


uint8_t calc_shift_value(const long param)
{
    if(param == 0){
        return (uint8_t)(0xff / 2);
    }
    if(param > 0){
        return (uint8_t)(0xff * 0.7f);
    }

    return (uint8_t)(0xff * 0.3f);
}

uint8_t calc_throttle_value(const long param)
{
    return (uint8_t)((0xff * param) / 100.0f);
}

uint8_t calc_rudder_value(const long param)
{
    return ((0xff * (500 - (param * 11))) / 1000.f);
}

const int g_arg_table_elements = 3;
const Argument g_arg_table[g_arg_table_elements] =
{
    {"shift", 1, -1, 1, calc_shift_value},
    {"throttle", 2, 0, 100, calc_throttle_value},
    {"rudder", 0, -40, 40, calc_rudder_value}
};

void print_usage()
{
    printf("Usage:\n");
    printf("control-ship <target> <parameter>\n");
    printf("   target: shift, throttle, rudder\n");
    printf("   parameter:\n");
    printf("       shift: 1 or 0 or -1 (1:Ahead, 0:Neutral, -1:Astern)\n");
    printf("       throttle: 0 to 100 (Percentage of output power)\n");
    printf("       rudder: -40 to 0 to 40 (>0:Starboard, 0:Center, <0:Port)\n");
}

void check_argc(const int argc)
{
    if(argc != 3){
        print_usage();
        exit(1);
    }
}

uint8_t parse_parameter(const char *parameter, int index){
    char *endptr;
    errno = 0;
    long param = strtol(parameter, &endptr, 10);
    if((errno != 0) || (*endptr !='\0')){
        print_usage();
        exit(3);
    }

    if((param < g_arg_table[index].parameter_min_) || (g_arg_table[index].parameter_max_ < param)){
        print_usage();
        exit(4);
    }

    return g_arg_table[index].calc_function_(param);
}

Command parse_argv(const char *argv[])
{
    for(int i = 0;i < g_arg_table_elements;++ i){
        if(strcmp(argv[1], g_arg_table[i].target_) == 0){
            uint8_t param = parse_parameter(argv[2], i);
            Command command = {g_arg_table[i].address_, param};
            return command;
        }
    }

    print_usage();
    exit(2);
}

int main(int argc, const char *argv[])
{
    check_argc(argc);
    Command command = parse_argv(argv);

    printf("Command 'control-ship(Target: %s(ch. %d), Parameter: %d)", argv[1], command.address_, command.value_[0]);

    if(i2c_write(0x08, command.address_, command.value_, 1) != 0){
        printf(" is failed to send.\n");
        exit(1);
    }
    printf(" is sended.\n");

    return 0;
}

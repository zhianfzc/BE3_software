#include "version.h"


const struct fw_misc_data g_fw_misc __attribute__((used, section("misc_data"))) = 
{
    .version[0] = 0,
    .version[1] = 5,
    .version[2] = 9,
    .version[3] = 0,
    .date = 20201031
};

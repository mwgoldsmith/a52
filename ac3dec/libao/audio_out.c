#include "config.h"

#include <inttypes.h>
#include <stdlib.h>

#include "ac3.h"
#include "audio_out.h"

extern ao_open_t ao_oss_open;
extern ao_open_t ao_null_open;
extern ao_open_t ao_null2_open;
extern ao_open_t ao_null4_open;
extern ao_open_t ao_float_open;

static ao_driver_t audio_out_drivers[] = {
#ifdef LIBAO_OSS
    {"oss", ao_oss_open},
#endif
    {"null", ao_null_open},
    {"null2", ao_null2_open},
    {"null4", ao_null4_open},
    {"float", ao_float_open},
    {NULL, NULL}
};

ao_driver_t * ao_drivers (void)
{
    return audio_out_drivers;
}

#include "config.h"

#include <inttypes.h>
#include <stdlib.h>

#include "ac3.h"
#include "audio_out.h"

extern ao_open_t ao_oss_open;
extern ao_open_t ao_ossdolby_open;
extern ao_open_t ao_oss4_open;
extern ao_open_t ao_oss6_open;
extern ao_open_t ao_solaris_open;
extern ao_open_t ao_solarisdolby_open;
extern ao_open_t ao_null_open;
extern ao_open_t ao_null4_open;
extern ao_open_t ao_null6_open;
extern ao_open_t ao_float_open;

static ao_driver_t audio_out_drivers[] = {
#ifdef LIBAO_OSS
    {"oss", ao_oss_open},
    {"ossdolby", ao_ossdolby_open},
    {"oss4", ao_oss4_open},
    {"oss6", ao_oss6_open},
#endif
#ifdef LIBAO_SOLARIS
    {"solaris", ao_solaris_open},
    {"solarisdolby", ao_solarisdolby_open},
#endif
    {"null", ao_null_open},
    {"null4", ao_null4_open},
    {"null6", ao_null6_open},
    {"float", ao_float_open},
    {NULL, NULL}
};

ao_driver_t * ao_drivers (void)
{
    return audio_out_drivers;
}

#include "kb.h"
#include "kb_output.h"
#include <string.h>
#ifdef SML_USE_TEST_DATA
#include "testdata.h"
int32_t td_index = 0;
#endif // SML_USE_TEST_DATA

#define SERIAL_OUT_CHARS_MAX 512

//static char serial_out_buf[SERIAL_OUT_CHARS_MAX];

const char *sml_class_label_from_raw_id(int32_t raw_id)
{
    // Knowledge Pack (current model) class map (from model_json/ClassMaps):
    //   0=Unknown, 1 = finger, 2 = noise, 3 = water
    switch (raw_id)
    {
    case 0:
        return "unknown";
    case 1:
        return "finger";
    case 2:
        return "noise";
    case 3:
        return "water";
    default:
        return "unknown";
    }
}

int32_t sml_recognition_run(int16_t *data, int32_t num_sensors)
{
    int32_t ret = -1;

    ret = kb_run_model(data, num_sensors, KB_MODEL_FINGER_NOISE_WATER_RANK_4_INDEX);
    
    if (ret >= 0)
    {
        kb_reset_model(KB_MODEL_FINGER_NOISE_WATER_RANK_4_INDEX);
    }
    
    return ret;
}
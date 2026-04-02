#ifndef __SML_RECOGNITION_RUN_H__
#define __SML_RECOGNITION_RUN_H__
#include "kb.h"

int32_t sml_recognition_run(int16_t *data, int32_t num_sensors);
const char *sml_class_label_from_raw_id(int32_t raw_id);

#endif //__SML_RECOGNITION_RUN_H__

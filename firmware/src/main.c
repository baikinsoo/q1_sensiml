/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include "kb.h"
#include "sml_recognition_run.h"


// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
extern volatile uint8_t measurement_done_touch;

/* Input value per channel (must match training):
 * 0=delta, 1=signal, 2=reference, 3=pseudo_signal_from_delta (clamped to training range)
 */
#define ML_INPUT_MODE 3

/* When ML_INPUT_MODE=3, map large deltas into the training-like range.
 * Example: training baseline ~512 and max ~550 => max delta ~38.
 */
#define TRAIN_BASELINE_SIGNAL 512
#define TRAIN_MAX_DELTA 70

static inline int16_t clamp_i16(int32_t v, int32_t lo, int32_t hi)
{
    if (v < lo)
        return (int16_t)lo;
    if (v > hi)
        return (int16_t)hi;
    return (int16_t)v;
}

/* Throttle UART logs: 20ms period (~50Hz). 25 -> ~2Hz, 50 -> ~1Hz */
#define LOG_EVERY_N_MEASUREMENTS 25u

#define ML_MAIN_LOOP_MODE_LEGACY_STREAM   0
#define ML_MAIN_LOOP_MODE_TOUCH_CAPTURE   1
#define ML_MAIN_LOOP_MODE                 ML_MAIN_LOOP_MODE_TOUCH_CAPTURE

/* Touch-start capture gating (used only when ML_MAIN_LOOP_MODE == TOUCH_CAPTURE):
 * - When |delta| crosses START threshold, flush model buffer and capture N samples.
 * - After capture completes, print ONE final label summary and wait until release.
 */
#define TOUCH_START_DELTA_THRESH 5
#define TOUCH_RELEASE_DELTA_THRESH 2
#define CAPTURE_NUM_SAMPLES 5u

int main(void)
{
    int16_t s0_signal = 0;
    int16_t s0_ref = 0;
    int16_t s0_delta = 0;

    int16_t s1_signal = 0;
    int16_t s1_ref = 0;
    int16_t s1_delta = 0;

    uint32_t meas_count = 0;
    
    const int32_t model_index = KB_MODEL_FINGER_NOISE_WATER_RANK_4_INDEX;
    
    int32_t expected_nsensors = -1;

    int16_t sample[2];
    int32_t ns_use = 1;
    int32_t ret = -1;

#if (ML_MAIN_LOOP_MODE == ML_MAIN_LOOP_MODE_TOUCH_CAPTURE)
    typedef enum
    {
        CAP_STATE_IDLE = 0,
        CAP_STATE_CAPTURE = 1,
        CAP_STATE_HOLD = 2,
    } cap_state_t;

    cap_state_t cap_state = CAP_STATE_IDLE;
    uint32_t capture_left = 0;
    int32_t last_valid_class = -1;
#endif

    SYS_Initialize(NULL);
    
    kb_model_init();
    
    expected_nsensors = kb_get_num_sensor_buffers(model_index);
    
    printf("ml_model_init,model_index=%ld,expected_nsensors=%ld,main_loop_mode=%d\r\n",
           (long)model_index,
           (long)expected_nsensors,
           (int)ML_MAIN_LOOP_MODE);

    while (true)
    {
        SYS_Tasks();

        touch_process();

        if (measurement_done_touch == 1)
        {
            measurement_done_touch = 0;
            meas_count++;

            // send sensor data right
            s0_signal = (int16_t)get_sensor_node_signal(0);
            s0_ref = (int16_t)get_sensor_node_reference(0);
            s0_delta = (int16_t)((int32_t)s0_signal - (int32_t)s0_ref);

            // send sensor data left
            s1_signal = (int16_t)get_sensor_node_signal(1);
            s1_ref = (int16_t)get_sensor_node_reference(1);
            s1_delta = (int16_t)((int32_t)s1_signal - (int32_t)s1_ref);

            ns_use = expected_nsensors;
            if (ns_use < 1)
            {
                ns_use = 1;
            }
            if (ns_use > 2)
            {
                ns_use = 2;
            }

            /* Choose input representation for each channel */
#if (ML_INPUT_MODE == 0)
            sample[0] = s0_delta;
            sample[1] = s1_delta;
#elif (ML_INPUT_MODE == 1)
            sample[0] = s0_signal;
            sample[1] = s1_signal;
#elif (ML_INPUT_MODE == 2)
            sample[0] = s0_ref;
            sample[1] = s1_ref;
#else /* ML_INPUT_MODE == 3 */
            {
                int32_t d0 = (int32_t)s0_delta;
                int32_t d1 = (int32_t)s1_delta;
                int16_t cd0 = clamp_i16(d0, 0, TRAIN_MAX_DELTA);
                int16_t cd1 = clamp_i16(d1, 0, TRAIN_MAX_DELTA);
                sample[0] = (int16_t)(TRAIN_BASELINE_SIGNAL + cd0);
                sample[1] = (int16_t)(TRAIN_BASELINE_SIGNAL + cd1);
            }
#endif

#if (ML_MAIN_LOOP_MODE == ML_MAIN_LOOP_MODE_LEGACY_STREAM)
            ret = sml_recognition_run(sample, ns_use);

            if ((meas_count % LOG_EVERY_N_MEASUREMENTS) == 0u)
            {
                printf("touch,ch0,signal=%d,ref=%d,delta=%d\r\n",
                       (int)s0_signal, (int)s0_ref, (int)s0_delta);
                printf("touch,ch1,signal=%d,ref=%d,delta=%d\r\n",
                       (int)s1_signal, (int)s1_ref, (int)s1_delta);

                if (ns_use == 2)
                {
                    printf("ml_vec,count=%lu,nsensors=2,ch0=%d,ch1=%d,ret=%ld\r\n",
                           (unsigned long)meas_count, (int)sample[0], (int)sample[1], (long)ret);
                }
                else
                {
                    printf("ml_vec,count=%lu,nsensors=1,ch0=%d,(ch1_ignored=%d),ret=%ld\r\n",
                           (unsigned long)meas_count, (int)sample[0], (int)sample[1], (long)ret);
                }

                if (ret >= 0)
                {
                    printf("--------------------\r\nml_label,%s\r\n--------------------\r\n",
                           sml_class_label_from_raw_id(ret));
                }
            }
#else
            /* ----- TOUCH CAPTURE: ???? ?? N?????? feed, ???? 1??, HOLD ----- */
            /* Touch-start detection uses delta regardless of ML_INPUT_MODE */
            int32_t abs_d0 = (s0_delta < 0) ? -(int32_t)s0_delta : (int32_t)s0_delta;
            int32_t abs_d1 = (s1_delta < 0) ? -(int32_t)s1_delta : (int32_t)s1_delta;
            bool touch_active = (abs_d0 >= TOUCH_RELEASE_DELTA_THRESH) || (abs_d1 >= TOUCH_RELEASE_DELTA_THRESH);
            bool touch_start = (abs_d0 >= TOUCH_START_DELTA_THRESH) || (abs_d1 >= TOUCH_START_DELTA_THRESH);

            ret = -1;

            switch (cap_state)
            {
            case CAP_STATE_IDLE:
                if (touch_start)
                {
                    kb_flush_model_buffer(model_index);
                    capture_left = CAPTURE_NUM_SAMPLES;
                    last_valid_class = -1;
                    cap_state = CAP_STATE_CAPTURE;
                }
                break;

            case CAP_STATE_CAPTURE:
                /* Stream exactly CAPTURE_NUM_SAMPLES after touch start */
                if (capture_left > 0u)
                {
                    ret = sml_recognition_run(sample, ns_use);
                    if (ret >= 0)
                    {
                        last_valid_class = ret;
                    }
                    capture_left--;
                }

                if (capture_left == 0u)
                {
                    /* Print ONE summary result and go to HOLD until release */
                    if (last_valid_class >= 0)
                    {
                        printf("--------------------\r\nml_label,%s\r\n--------------------\r\n",
                               sml_class_label_from_raw_id(last_valid_class));
                    }
                    else
                    {
                        printf("--------------------\r\nml_label,none\r\n--------------------\r\n");
                    }
                    cap_state = CAP_STATE_HOLD;
                }
                break;

            case CAP_STATE_HOLD:
                /* Wait until touch is released before allowing another capture */
                if (!touch_active)
                {
                    cap_state = CAP_STATE_IDLE;
                }
                break;
            }

            /* Reduce console spam: log only while CAPTURE is active */
            if ((cap_state == CAP_STATE_CAPTURE) && ((meas_count % LOG_EVERY_N_MEASUREMENTS) == 0u))
            {
                printf("touch,ch0,signal=%d,ref=%d,delta=%d\r\n",
                       (int)s0_signal, (int)s0_ref, (int)s0_delta);
                printf("touch,ch1,signal=%d,ref=%d,delta=%d\r\n",
                       (int)s1_signal, (int)s1_ref, (int)s1_delta);

                if (ns_use == 2)
                {
                    printf("ml_vec,count=%lu,nsensors=2,ch0=%d,ch1=%d,ret=%ld\r\n",
                           (unsigned long)meas_count, (int)sample[0], (int)sample[1], (long)ret);
                }
                else
                {
                    printf("ml_vec,count=%lu,nsensors=1,ch0=%d,(ch1_ignored=%d),ret=%ld\r\n",
                           (unsigned long)meas_count, (int)sample[0], (int)sample[1], (long)ret);
                }

                printf("cap_state=%d,capture_left=%lu,last_valid=%ld,ml_input_mode=%d\r\n",
                       (int)cap_state,
                       (unsigned long)capture_left,
                       (long)last_valid_class,
                       (int)ML_INPUT_MODE);
            }
#endif
        }
    }
    return (EXIT_FAILURE);
}

/*******************************************************************************
 End of File
*/

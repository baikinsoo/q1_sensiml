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
 * 0=delta, 1=signal, 2=reference, 3=signal from ref+delta (same as 1 if no overflow)
 */
#define ML_INPUT_MODE 3

/* Throttle UART logs: 20ms period (~50Hz). 25 -> ~2Hz, 50 -> ~1Hz */
#define LOG_EVERY_N_MEASUREMENTS 25u

/* Window capture (delta = signal - reference from touch library):
 * COLLECT: shift window, append current ML sample each tick (only this state writes the buffer).
 * When window_fills >= WINDOW_LEN and |delta| >= WINDOW_DELTA_THRESH on either channel,
 *   fall through to RUNNING in the same tick (buffer is not updated again until COLLECT resumes).
 * RUNNING: kb_flush, replay window[0..WINDOW_LEN-1], print ml_label, clear window, -> HOLD.
 * HOLD: wait until both |delta| < TOUCH_RELEASE_DELTA_THRESH (no buffer updates), -> COLLECT.
 */
#define WINDOW_LEN 20u
#define WINDOW_DELTA_THRESH 20
#define TOUCH_RELEASE_DELTA_THRESH 2

// ?? ?? 1. ? ?? 2. ?? ? 3. ?? ??? ???? ??
typedef enum
{
    CAP_STATE_COLLECT = 0,
    CAP_STATE_RUNNING = 1,
    CAP_STATE_HOLD = 2,
} cap_state_t;

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

    cap_state_t cap_state = CAP_STATE_COLLECT;
    uint32_t window_fills = 0;
    int32_t last_valid_class = -1;
    int16_t window[WINDOW_LEN][2];
    uint32_t wi;

    SYS_Initialize(NULL);

    kb_model_init();

    // ?? ??? ??? ?? ?
    expected_nsensors = kb_get_num_sensor_buffers(model_index);

    printf("ml_model_init,model_index=%ld,expected_nsensors=%ld,mode=touch_capture\r\n",
           (long)model_index,
           (long)expected_nsensors);

    for (wi = 0u; wi < WINDOW_LEN; wi++)
    {
        window[wi][0] = 0;
        window[wi][1] = 0;
    }
    printf("window_capture,len=%u,delta_thresh=%d,ml_input_mode=%d,states=collect,running,hold\r\n",
           (unsigned)WINDOW_LEN,
           (int)WINDOW_DELTA_THRESH,
           (int)ML_INPUT_MODE);

    while (true)
    {
        SYS_Tasks();

        touch_process();

        if (measurement_done_touch == 1)
        {
            measurement_done_touch = 0;
            meas_count++;

            // ?? ?? ?, ?? ?, ?? ?? ?? ??.
            s0_signal = (int16_t)get_sensor_node_signal(0);
            s0_ref = (int16_t)get_sensor_node_reference(0);
            s0_delta = (int16_t)((int32_t)s0_signal - (int32_t)s0_ref);

            // ?? ?? ?, ?? ?, ?? ?? ?? ??.
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

#if (ML_INPUT_MODE == 0)
            sample[0] = s0_delta;
            sample[1] = s1_delta;
#elif (ML_INPUT_MODE == 1)
            sample[0] = s0_signal;
            sample[1] = s1_signal;
#elif (ML_INPUT_MODE == 2)
            sample[0] = s0_ref;
            sample[1] = s1_ref;
#else /* ML_INPUT_MODE == 3: ref + delta, no clamp (same as signal if in int16 range) */
            
            // ?? ???? ???? ? ? ? ?? ?? 20? ?? ?? ??? ?? ???.
            sample[0] = (int16_t)((int32_t)s0_ref + (int32_t)s0_delta);
            sample[1] = (int16_t)((int32_t)s1_ref + (int32_t)s1_delta);
#endif

            // ?? ???? ???? ?? ?? ????? ???? ??
            // ?? ??? ??? threshold ? ???? ??? ??? ?? ?? ??? ???? ?? ??? ??? ????
            /* Trigger/HOLD use raw delta vs reference (signal - ref). */
            int32_t abs_d0 = (s0_delta < 0) ? -(int32_t)s0_delta : (int32_t)s0_delta;
            int32_t abs_d1 = (s1_delta < 0) ? -(int32_t)s1_delta : (int32_t)s1_delta;
            bool touch_active = (abs_d0 >= TOUCH_RELEASE_DELTA_THRESH) || (abs_d1 >= TOUCH_RELEASE_DELTA_THRESH);
            bool window_cross = (abs_d0 >= WINDOW_DELTA_THRESH) || (abs_d1 >= WINDOW_DELTA_THRESH);

            ret = -1;

            switch (cap_state)
            {
                // ??? ???? ??
            case CAP_STATE_COLLECT:
                for (wi = 0u; wi < WINDOW_LEN - 1u; wi++)
                {
                    // 
                    window[wi][0] = window[wi + 1u][0];
                    window[wi][1] = window[wi + 1u][1];
                }
                window[WINDOW_LEN - 1u][0] = sample[0];
                window[WINDOW_LEN - 1u][1] = sample[1];
                if (window_fills < 0xFFFFFFFFu)
                {
                    window_fills++;
                }

                if (!(window_fills >= WINDOW_LEN && window_cross))
                {
                    break;
                }
                /* Fall through: freeze buffer after this sample; run model same tick. */

            case CAP_STATE_RUNNING:
                kb_flush_model_buffer(model_index);
                last_valid_class = -1;
                for (wi = 0u; wi < WINDOW_LEN; wi++)
                {
                    ret = sml_recognition_run(&window[wi][0], ns_use);
                    if (ret >= 0)
                    {
                        last_valid_class = ret;
                    }
                }
                if (last_valid_class >= 0)
                {
                    printf("--------------------\r\nml_label,%s\r\n--------------------\r\n",
                           sml_class_label_from_raw_id(last_valid_class));
                }
                else
                {
                    printf("--------------------\r\nml_label,none\r\n--------------------\r\n");
                }
                for (wi = 0u; wi < WINDOW_LEN; wi++)
                {
                    window[wi][0] = 0;
                    window[wi][1] = 0;
                }
                window_fills = 0u;
                cap_state = CAP_STATE_HOLD;
                break;

            case CAP_STATE_HOLD:
                if (!touch_active)
                {
                    cap_state = CAP_STATE_COLLECT;
                }
                break;
            }

            if ((cap_state == CAP_STATE_COLLECT) && ((meas_count % LOG_EVERY_N_MEASUREMENTS) == 0u))
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

                printf("cap_state=%d,window_fills=%lu,last_valid=%ld,ml_input_mode=%d\r\n",
                       (int)cap_state,
                       (unsigned long)window_fills,
                       (long)last_valid_class,
                       (int)ML_INPUT_MODE);
            }
        }
    }
    return (EXIT_FAILURE);
}

/*******************************************************************************
 End of File
*/

/* USB Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

// DESCRIPTION:
// This example contains minimal code to make ESP32-S2 based device
// recognizable by USB-host devices as a USB Serial Device.

#include <stdint.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "sdkconfig.h"
#include "tusb_hid.h"

#define TAG "HID"

//#define CONFIG_TOUCH_MOUSE 1
#define CONFIG_TOUCH_KEYBOAD

#ifdef CONFIG_TOUCH_MOUSE
#include "touch_element/touch_slider.h"
#include "touch_element/touch_board.h"
touch_board_handle_t board_handle = NULL;
#elif defined CONFIG_TOUCH_KEYBOAD
#include "touch_element/touch_matrix.h"
#endif

#define TOUCH_SLIDER1_SUM    7
#define TOUCH_SLIDER2_SUM    6

#define X_AXIS_CHANNEL_NUM     4
#define Y_AXIS_CHANNEL_NUM     3

#define SENSOR_PERIOD 10

static touch_matrix_handle_t matrix_handle;

#ifdef CONFIG_TOUCH_KEYBOAD
/* Touch Matrix Button x-axis channels array */
static const touch_pad_t x_axis_channel[X_AXIS_CHANNEL_NUM] = {
    TOUCH_PAD_NUM12,
    TOUCH_PAD_NUM13,
    TOUCH_PAD_NUM4,
    TOUCH_PAD_NUM2,
};

static const touch_pad_t y_axis_channel[Y_AXIS_CHANNEL_NUM] = {
    TOUCH_PAD_NUM6,
    TOUCH_PAD_NUM8,
    TOUCH_PAD_NUM10,
};

static const float x_axis_channel_sens[X_AXIS_CHANNEL_NUM] = {
    0.2F,
    0.2F,
    0.2F,
    0.2F
};

static const float y_axis_channel_sens[Y_AXIS_CHANNEL_NUM] = {
    0.2F,
    0.2F,
    0.2F
};

/* Matrix callback routine */
void matrix_handler(touch_matrix_handle_t out_handle, touch_matrix_message_t *out_message, void *arg)
{
    (void) arg; //Unused
    uint8_t _keycode[6] = { 0 };

    if (out_handle != matrix_handle) {
        return;
    }

    if (out_message->event == TOUCH_MATRIX_EVT_ON_PRESS) {
        ESP_LOGI(TAG, "Matrix Press, axis: (%d, %d) index: %d", out_message->position.x_axis, out_message->position.y_axis, out_message->position.index);

        switch (out_message->position.index) {
            case 6:
                _keycode[0] = HID_KEY_KEYPAD_1;
                break;

            case 7:
                _keycode[0] = HID_KEY_KEYPAD_2;
                break;

            case 8:
                _keycode[0] = HID_KEY_KEYPAD_3;
                break;

            case 3:
                _keycode[0] = HID_KEY_KEYPAD_4;
                break;

            case 4:
                _keycode[0] = HID_KEY_KEYPAD_5;
                break;

            case 5:
                _keycode[0] = HID_KEY_KEYPAD_6;
                break;

            case 0:
                _keycode[0] = HID_KEY_KEYPAD_7;
                break;

            case 1:
                _keycode[0] = HID_KEY_KEYPAD_8;
                break;

            case 2:
                _keycode[0] = HID_KEY_KEYPAD_9;
                break;

            case 9:
                _keycode[0] = HID_KEY_KEYPAD_0;
                break;

            case 10:
                _keycode[0] = HID_KEY_KEYPAD_DECIMAL;
                break;

            case 11:
                _keycode[0] = HID_KEY_KEYPAD_ENTER;
                break;

            default:
                _keycode[0] = HID_KEY_NONE;
                break;
        }

    tinyusb_hid_keyboard_report(_keycode);
    } else if (out_message->event == TOUCH_MATRIX_EVT_ON_RELEASE) {
        ESP_LOGI(TAG, "Matrix Release, axis: (%d, %d) index: %d", out_message->position.x_axis, out_message->position.y_axis, out_message->position.index);
    } else if (out_message->event == TOUCH_MATRIX_EVT_ON_LONGPRESS) {
        ESP_LOGI(TAG, "Matrix LongPress, axis: (%d, %d) index: %d", out_message->position.x_axis, out_message->position.y_axis, out_message->position.index);
    }
}

#endif

#ifdef CONFIG_TOUCH_MOUSE
static const touch_pad_t channel1_array[TOUCH_SLIDER1_SUM] = {
    TOUCH_PAD_NUM1,
    TOUCH_PAD_NUM3,
    TOUCH_PAD_NUM5,
    TOUCH_PAD_NUM7,
    TOUCH_PAD_NUM9,
    TOUCH_PAD_NUM11,
    TOUCH_PAD_NUM13,
};

/**
 * Please refer to the slider sensitivity geter method
 * TODO: add graphical debug tool to analyse/generate the sensitivity
 */
static const float channel1_sens_array[TOUCH_SLIDER1_SUM] = {
    0.115F,
    0.115F,
    0.115F,
    0.115F,
    0.115F,
    0.115F,
    0.115F
};

static const touch_pad_t channel2_array[TOUCH_SLIDER2_SUM] = {
    TOUCH_PAD_NUM8,
    TOUCH_PAD_NUM6,
    TOUCH_PAD_NUM4,
    TOUCH_PAD_NUM2,
    TOUCH_PAD_NUM10,
    TOUCH_PAD_NUM12,
};

static const float channel2_sens_array[TOUCH_SLIDER2_SUM] = {
    0.115F,
    0.115F,
    0.115F,
    0.115F,
    0.115F,
    0.115F
};

/* touch_board callback routine */
static void touch_board_handler(touch_board_handle_t out_handle, touch_board_message_t *out_message, void *arg)
{
    static int8_t last_press_x_axis = 0;
    static int8_t last_press_y_axis = 0;
    static int64_t timestamp_us = 0;//esp_timer_get_time();
    static int click_times = 0;

    if (out_handle != board_handle) {
        return;
    }

    if (out_message->event == TOUCH_BOARD_EVT_ON_PRESS) {
        last_press_x_axis = out_message->position.x_axis;
        last_press_y_axis = out_message->position.y_axis;
        timestamp_us = esp_timer_get_time();
        ESP_LOGD(TAG, "on press event: %d   %d\n", out_message->position.x_axis, out_message->position.y_axis);
        /* code */
    } else if (out_message->event == TOUCH_BOARD_EVT_ON_RELEASE) {
        int64_t timestamp_bias_us = esp_timer_get_time() - timestamp_us;

        if (timestamp_bias_us > 0 && timestamp_bias_us < 250000 && timestamp_bias_us > 80000) {
            click_times++;
            tinyusb_hid_mouse_button_report(0x01);
            timestamp_us = esp_timer_get_time();
            has_touch_board_key = true;
            printf("click_times = %d\n", click_times);
        } else {
            click_times = 0;
        }

        if (click_times > 1) {
            printf("mutil click = %d\n", click_times);
            //tinyusb_hid_mouse_button_report(0x03);
            click_times = 0;
        }

        last_press_x_axis = 0;
        last_press_y_axis = 0;
        ESP_LOGD(TAG, "on release event: %d   %d\n", out_message->position.x_axis, out_message->position.y_axis);
    } else {
        int64_t timestamp_bias_us = esp_timer_get_time() - timestamp_us;

        if (timestamp_bias_us > 400000) {
            last_press_x_axis = out_message->position.x_axis;
            last_press_y_axis = out_message->position.y_axis;
            timestamp_us = esp_timer_get_time();
            ESP_LOGD(TAG, "timeout \n");
        }

        int8_t x = (out_message->position.x_axis - last_press_x_axis) / 2;
        int8_t y = (out_message->position.y_axis - last_press_y_axis) / 2;

        if (abs(x) > 0 || abs(y) > 0) {
            tinyusb_hid_mouse_move_report(x, y, 0, 0);
        }

        ESP_LOGD(TAG, "x=%d y=%d on calculation event: %d   %d\n", last_press_x_axis, last_press_y_axis, out_message->position.x_axis, out_message->position.y_axis);
    }

}
#endif

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
    BLINK_NOT_MOUNTED = 250,
    BLINK_MOUNTED = 1000,
    BLINK_SUSPENDED = 2500,
};


void app_main(void)
{
    ESP_LOGI(TAG, "USB initialization");
    tinyusb_config_t tusb_cfg = {}; // the configuration using default values
    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

#ifdef CONFIG_TOUCH_KEYBOAD
    touch_elem_global_config_t global_config = TOUCH_ELEM_GLOBAL_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(touch_element_install(&global_config));
    ESP_LOGI(TAG, "Touch element library installed");

    touch_matrix_global_config_t matrix_global_config = TOUCH_MATRIX_GLOBAL_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(touch_matrix_install(&matrix_global_config));
    ESP_LOGI(TAG, "Touch matrix installed");
    /* Create Touch Matrix Button */
    touch_matrix_config_t matrix_config = {
        .x_channel_array = x_axis_channel,
        .y_channel_array = y_axis_channel,
        .x_sensitivity_array = x_axis_channel_sens,
        .y_sensitivity_array = y_axis_channel_sens,
        .x_channel_num = (sizeof(x_axis_channel) / sizeof(x_axis_channel[0])),
        .y_channel_num = (sizeof(y_axis_channel) / sizeof(y_axis_channel[0]))
    };
    ESP_ERROR_CHECK(touch_matrix_create(&matrix_config, &matrix_handle));
    /* Subscribe touch matrix events (On Press, On Release, On LongPress) */
    ESP_ERROR_CHECK(touch_matrix_subscribe_event(matrix_handle, TOUCH_ELEM_EVENT_ON_PRESS | TOUCH_ELEM_EVENT_ON_RELEASE | TOUCH_ELEM_EVENT_ON_LONGPRESS, NULL));
    /* Set CALLBACK as the dispatch method */
    ESP_ERROR_CHECK(touch_matrix_set_dispatch_method(matrix_handle, TOUCH_ELEM_DISP_CALLBACK));
    /* Register a handler function to handle event messages */
    ESP_ERROR_CHECK(touch_matrix_set_callback(matrix_handle, matrix_handler));
    ESP_LOGI(TAG, "Touch matrix created");

    touch_element_start();
    ESP_LOGI(TAG, "Touch element library start");
#elif CONFIG_TOUCH_MOUSE
    touch_elem_global_config_t global_config = TOUCH_ELEM_GLOBAL_DEFAULT_CONFIG();
    global_config.software.processing_period = 2;
    touch_element_install(&global_config);

    touch_board_global_config_t board_global_config = {
        .horizontal_global_config =  TOUCH_SLIDER_GLOBAL_DEFAULT_CONFIG(),
        .vertical_global_config = TOUCH_SLIDER_GLOBAL_DEFAULT_CONFIG()
    };
    ESP_ERROR_CHECK(touch_board_install(&board_global_config));

    touch_board_config_t board_config = {
        .horizontal_config.channel_array = channel1_array,
        .horizontal_config.sensitivity_array = channel1_sens_array,
        .horizontal_config.channel_num = TOUCH_SLIDER1_SUM,
        .horizontal_config.position_range = 100,
        .vertical_config.channel_array = channel2_array,
        .vertical_config.sensitivity_array = channel2_sens_array,
        .vertical_config.channel_num = TOUCH_SLIDER2_SUM,
        .vertical_config.position_range = 100,
    };
    ESP_ERROR_CHECK(touch_board_create(&board_config, &board_handle));
    ESP_ERROR_CHECK(touch_board_subscribe_event(board_handle, TOUCH_ELEM_EVENT_ON_PRESS | TOUCH_ELEM_EVENT_ON_RELEASE | TOUCH_ELEM_EVENT_ON_CALCULATION, NULL));
    ESP_ERROR_CHECK(touch_board_set_dispatch_method(board_handle, TOUCH_ELEM_DISP_CALLBACK));
    ESP_ERROR_CHECK(touch_board_set_callback(board_handle, touch_board_handler));
    ESP_ERROR_CHECK(touch_element_start()); /*start a sensor, data ready events will be posted once data acquired successfully*/
#endif

    ESP_LOGI(TAG, "USB initialization DONE");
}

#include <stdio.h>
#include "pico/stdlib.h"
#include "ei_run_classifier.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"

#define I2C_PORT i2c0
#define CONVERT_G_TO_MS2    9.80665f
#define FREQUENCY_HZ        50
#define INTERVAL_MS         (1000 / (FREQUENCY_HZ + 1))

static int addr = 0x1d;

static float features[258];
const uint LED_PIN = 25;

void accel_init(void)
{
    sleep_ms(1000);
    uint8_t deviceIDReg = 0x0D;
    uint8_t chipID[1];
    i2c_write_blocking(I2C_PORT, addr, &deviceIDReg, 1, true);
    i2c_read_blocking(I2C_PORT, addr, chipID, 1, false);

    if(chipID[0] != 0x1A){
        printf("correct value: %d", chipID[0]);
        while(1){
            printf("Chip ID Not Correct - Check Connection!");
            sleep_ms(5000);
        }
    }    
}

void set_I2C_Active()
{
    uint8_t status[1];
    uint8_t activeMask = 0x01;
    uint8_t Ctrl_Reg1_Addr = 0x2A;
    i2c_write_blocking(I2C_PORT, addr, &Ctrl_Reg1_Addr, 1, true);   // read from ctro_reg1
    i2c_read_blocking(I2C_PORT, addr, status, 1, false);            // read the current status code
    status[0] = status[0] | activeMask;         // set the status to stanby mode
    uint8_t data_input[2];                      // prepare the array for writing I2C
    data_input[0] = Ctrl_Reg1_Addr;
    data_input[1] = status[0];
    i2c_write_blocking(I2C_PORT, addr, data_input, 2, false);
}

int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {

    ei_printf("Gathering data \n");

    uint8_t accel[6];                   // Store data from the 6 acceleration registers (2 regs for each direction)
    int16_t accelX, accelY, accelZ;     // Combined 3 axis data
    uint8_t accelRegAddr = 0x01;        // Start register address

    for (uint8_t i = 0; i < (258/3); i = i + 3) 
    {
        i2c_write_blocking(I2C_PORT, addr, &accelRegAddr, 1, true);     // start from X_MSB
        i2c_read_blocking(I2C_PORT, addr, accel, 6, false);             // read all 6 values

        accelX = ((accel[0]<<8) | accel[1]);
        accelY = ((accel[2]<<8) | accel[3]);
        accelZ = ((accel[4]<<8) | accel[5]);

        accelX = accelX >> 2;   // trim the tailing two 0's
        accelY = accelY >> 2;           
        accelZ = accelZ >> 2;

        features[i] = accelX / 2048.00f;
        features[i+1] = accelY / 2048.00f;
        features[i+2] = accelZ / 2048.00f;
        //ei_printf("%.5f\n", features[i]);
        sleep_ms(INTERVAL_MS);
    }
    
    memcpy(out_ptr, features + offset, length * sizeof(float));
    return 0;
}

int main()
{
    stdio_init_all();
    // adc_init();

    // adc_gpio_init(26);
    // adc_select_input(0);
    
    sleep_ms(15000);

    // I2C
    printf("Set i2c port!");
    i2c_init(I2C_PORT, 400000);
    gpio_set_function(12, GPIO_FUNC_I2C);
    gpio_set_function(13, GPIO_FUNC_I2C);
    gpio_pull_up(12);
    gpio_pull_up(13);

    accel_init();

    set_I2C_Active();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    gpio_put(LED_PIN, 1);
    sleep_ms(250);
    gpio_put(LED_PIN, 0);
    sleep_ms(250);

    if (sizeof(features) / sizeof(float) != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
        ei_printf("The size of your 'features' array is not correct. Expected %lu items, but had %lu\n",
            EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, sizeof(features) / sizeof(float));
        sleep_ms(1000);
        return -1;
    }

    ei_impulse_result_t result = {0};       // check ei_classifier_types.h for more info

    // the features are stored into flash, and we don't want to load everything into RAM
    signal_t features_signal;
    features_signal.total_length = sizeof(features) / sizeof(features[0]);
    features_signal.get_data = &raw_feature_get_data;

    while (true) 
    
    {
        ei_printf("Edge Impulse standalone inferencing (Raspberry Pico 2040)\n");
        gpio_put(LED_PIN, 0);
        // invoke the impulse
        EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false /* debug */);
        ei_printf("run_classifier returned: %d\n", res);

        if (res != 0) return res;

    //     // print the predictions
    //     ei_printf("Predictions ");
    //     ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
    //         result.timing.dsp, result.timing.classification, result.timing.anomaly);
    //     ei_printf(": \n");
    //     ei_printf("[");
    //     for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    //         ei_printf("%.5f", result.classification[ix].value);
    // #if EI_CLASSIFIER_HAS_ANOMALY == 1
    //         ei_printf(", ");
    // #else
    //         if (ix != EI_CLASSIFIER_LABEL_COUNT - 1) {
    //             ei_printf(", ");
    //         }
    // #endif
    //     }
    // #if EI_CLASSIFIER_HAS_ANOMALY == 1
    //     ei_printf("%.3f", result.anomaly);
    // #endif
    //     ei_printf("]\n");

    
        const char *currLabel = "";
        float confLevel = 0;
        // human-readable predictions
        for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
            ei_printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
            if (result.classification[ix].value > confLevel)
            {
                confLevel = result.classification[ix].value;
                currLabel = result.classification[ix].label;
            }
        }
    #if EI_CLASSIFIER_HAS_ANOMALY == 1
        ei_printf("    anomaly score: %.3f\n", result.anomaly);
    #endif
        printf("Curr motion: %s", currLabel);
        gpio_put(LED_PIN, 1);
        sleep_ms(1000);

    }
return 0;
}

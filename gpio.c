#include "hs_serial.h"

int gpio_values[NUMBER_OF_GPIO];
mraa_gpio_context gpio_contexts[NUMBER_OF_GPIO];

void set_gpio_value(int pin, int value) {  
    mraa_gpio_write(gpio_contexts[pin], value);
    gpio_values[pin]=value;
}

void toggle_gpio_value(int pin) {
    set_gpio_value(pin, !gpio_values[pin]);
}

void init_gpio(){
    int i;

        gpio_contexts[0] = mraa_gpio_init(7);   //claim pins
        mraa_gpio_use_mmaped(gpio_contexts[0], 1); //Enable using memory mapped io instead of sysfs
        mraa_gpio_dir(gpio_contexts[0], MRAA_GPIO_OUT); //pin 7 = output
        set_gpio_value(0,0);                    //start at 0
        gpio_contexts[1] = mraa_gpio_init(2);   //claim pins
        mraa_gpio_use_mmaped(gpio_contexts[1], 1); //Enable using memory mapped io instead of sysfs
        mraa_gpio_dir(gpio_contexts[1], MRAA_GPIO_OUT); //pin 2 = output
        set_gpio_value(1,0);                    //start at 0
        
    
}


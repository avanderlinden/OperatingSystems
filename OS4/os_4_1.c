#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/kernel.h>

#define LED1 4
#define LED2 17
#define LED3 27
#define LED4 22

#define HIGH 1
#define LOW 0

/*
 * Struct gpio_config_leds is a configuration for the
 * GPIOs that are connected to the TU Delft LED module.
 */
static struct gpio gpio_config_leds[] = {
        { LED1, GPIOF_OUT_INIT_LOW,  "LED 1" },
        { LED2, GPIOF_OUT_INIT_LOW,  "LED 2" },
        { LED3, GPIOF_OUT_INIT_LOW,  "LED 3" },
        { LED4, GPIOF_OUT_INIT_LOW,  "LED 4" },
    };


/*
 * init_module is called when module is loaded
 *
 * Requests the GPIOs for the TU Delft LED module
 * and turns on the leds
 */
int init_module(void)
{
    int status = 0;

    MODULE_LICENSE("GPL");
    MODULE_AUTHOR("Alex van der Linden");
    MODULE_DESCRIPTION("Turn on LEDs");
    MODULE_VERSION("0.1");

    printk(KERN_INFO "Lights on!\n");

    status = gpio_request_array(gpio_config_leds, ARRAY_SIZE(gpio_config_leds));
    if(status){
        return 1;
    }

    gpio_set_value(LED1, HIGH);
    gpio_set_value(LED2, HIGH);
    gpio_set_value(LED3, HIGH);
    gpio_set_value(LED4, HIGH);

    return 0;

}

/*
 * cleanup_module is called when module is unloaded
 *
 * Turns off the LEDs and release the GPIOs
 */
void cleanup_module(void)
{
    printk(KERN_INFO "Lights off!\n");

    gpio_set_value(LED1, LOW);
    gpio_set_value(LED2, LOW);
    gpio_set_value(LED3, LOW);
    gpio_set_value(LED4, LOW);

    gpio_free_array(gpio_config_leds, ARRAY_SIZE(gpio_config_leds));

}


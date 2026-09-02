
/*
 * Viikkotehtävä 1
 * Tommi Räisänen TVT24SPL
 * Tavoittelen täysiä pisteitä ja mielestäni sain kaikki tehtävät tehtyä onnistuneesti
 */


#include <zephyr/kernel.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

// Red led thread initialization
#define STACKSIZE 500
#define PRIORITY 5
// Configure buttons
#define BUTTON_0 DT_ALIAS(sw0)
#define BUTTON_1 DT_ALIAS(sw1)
#define BUTTON_2 DT_ALIAS(sw2)
#define BUTTON_3 DT_ALIAS(sw3)
#define BUTTON_4 DT_ALIAS(sw4)

// Led pin configurations
static const struct gpio_dt_spec red = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec blue = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);

// #define BUTTON_1 DT_ALIAS(sw1)
static const struct gpio_dt_spec button_0 = GPIO_DT_SPEC_GET_OR(BUTTON_0, gpios, {0});
static const struct gpio_dt_spec button_1 = GPIO_DT_SPEC_GET_OR(BUTTON_1, gpios, {0});
static const struct gpio_dt_spec button_2 = GPIO_DT_SPEC_GET_OR(BUTTON_2, gpios, {0});
static const struct gpio_dt_spec button_3 = GPIO_DT_SPEC_GET_OR(BUTTON_3, gpios, {0});
static const struct gpio_dt_spec button_4 = GPIO_DT_SPEC_GET_OR(BUTTON_4, gpios, {0});

static struct gpio_callback button_0_data;
static struct gpio_callback button_1_data;
static struct gpio_callback button_2_data;
static struct gpio_callback button_3_data;
static struct gpio_callback button_4_data;



int led_state = 0;
// led_state: 0 = punainen
// led_state: 1 = keltainen
// led_state: 2 = vihreä
// led_state: 4 = pause

int check_led_state = 0;


/**  extra tehtäviä varten **/
int red_led_state = 0;
int yellow_led_state = 0;
int green_led_state = 0;
int yellow_blinky_state = 0;

// Button interrupt handler
void button_0_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{   // pause kytkin
	printk("Button 0 pressed\n");
	
	if(led_state != 4) {
	check_led_state = led_state;
	led_state = 4;
	} else {
		led_state = check_led_state;
	}
}

void button_1_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	// punanen led kytkin
	printk("Button 1 pressed\n");
	// nollataan muut 
	gpio_pin_set_dt(&green, 0);
	yellow_led_state = 0;
	green_led_state = 0;
	yellow_blinky_state = 0;

	if(red_led_state == 0) {
		red_led_state = 1;
		gpio_pin_set_dt(&red, 1);

	} else {
		gpio_pin_set_dt(&red, 0);
		red_led_state = 0;
	}
}

void button_2_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	printk("Button 2 pressed\n");
	// keltainen ledi kytkin

	// nollataan muut 
	red_led_state = 0;
	green_led_state = 0;
	yellow_blinky_state = 0;

	if(yellow_led_state == 0) {
		yellow_led_state = 1;
		gpio_pin_set_dt(&red, 1);
		gpio_pin_set_dt(&green, 1);

	} else {
		gpio_pin_set_dt(&red, 0);
		gpio_pin_set_dt(&green, 0);
		yellow_led_state = 0;
	}
}

void button_3_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	printk("Button 3 pressed\n");
	//vihreä ledi kytkin
	// nollataan muut
	gpio_pin_set_dt(&red, 0);
	red_led_state = 0;
	yellow_led_state = 0;
	yellow_blinky_state = 0;

	if(green_led_state == 0) {
		green_led_state = 1;
		gpio_pin_set_dt(&green, 1);

	} else {
		gpio_pin_set_dt(&green, 0);
		green_led_state = 0;
	}
}

void button_4_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	//keltanen vlikkuu
	printk("Button 4 pressed\n");
	//nollataan muut 
	red_led_state = 0;
	yellow_led_state = 0;
	green_led_state = 0;


	if(yellow_blinky_state == 0) { 
		yellow_blinky_state = 1; 
	} else {
		yellow_blinky_state = 0;
	}
	
}



void red_led_task(void *, void *, void*);
void green_led_task(void *, void *, void*);
void blue_led_task(void *, void *, void*);
void yellow_led_task(void *, void *, void*);
void yellow_blinky_task(void *, void *, void*);




K_THREAD_DEFINE(red_thread,STACKSIZE,red_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(green_thread,STACKSIZE,green_led_task,NULL,NULL,NULL,PRIORITY,0,0);
//K_THREAD_DEFINE(blue_thread,STACKSIZE,blue_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(yellow_thread,STACKSIZE,yellow_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(yellow_blinky_thread,STACKSIZE,yellow_blinky_task,NULL,NULL,NULL,PRIORITY,0,0);



int init_led() {

	// Led pin initialization
	int ret = gpio_pin_configure_dt(&red, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Error: red Led configure failed\n");		
		return ret;
	}

	ret = gpio_pin_configure_dt(&green, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Error: green Led configure failed\n");		
		return ret;
	}

	ret = gpio_pin_configure_dt(&blue, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Error: blue Led configure failed\n");		
		return ret;
	}


	// set led off
	gpio_pin_set_dt(&red,   0);
	gpio_pin_set_dt(&green, 0);
	gpio_pin_set_dt(&blue,  0);

	printk("Leds initialized ok\n");
	
	return 0;
}

// Button initialization
int init_button() {

	int ret;
	if (!gpio_is_ready_dt(&button_0)) {
		printk("Error: button 0 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_0, GPIO_INPUT);
	if (ret != 0) {
		printk("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_0, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	gpio_init_callback(&button_0_data, button_0_handler, BIT(button_0.pin));
	gpio_add_callback(button_0.port, &button_0_data);
	printk("Set up button 0 ok\n");


	if (!gpio_is_ready_dt(&button_1)) {
		printk("Error: button 1 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_1, GPIO_INPUT);
	if (ret != 0) {
		printk("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_1, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	gpio_init_callback(&button_1_data, button_1_handler, BIT(button_1.pin));
	gpio_add_callback(button_1.port, &button_1_data);
	printk("Set up button 1 ok\n");



	if (!gpio_is_ready_dt(&button_2)) {
		printk("Error: button 2 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_2, GPIO_INPUT);
	if (ret != 0) {
		printk("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_2, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	gpio_init_callback(&button_2_data, button_2_handler, BIT(button_2.pin));
	gpio_add_callback(button_2.port, &button_2_data);
	printk("Set up button 2 ok\n");



	if (!gpio_is_ready_dt(&button_3)) {
		printk("Error: button 3 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_3, GPIO_INPUT);
	if (ret != 0) {
		printk("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_3, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	gpio_init_callback(&button_3_data, button_3_handler, BIT(button_3.pin));
	gpio_add_callback(button_3.port, &button_3_data);
	printk("Set up button 3 ok\n");


	if (!gpio_is_ready_dt(&button_4)) {
		printk("Error: button 4 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_4, GPIO_INPUT);
	if (ret != 0) {
		printk("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_4, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	gpio_init_callback(&button_4_data, button_4_handler, BIT(button_4.pin));
	gpio_add_callback(button_4.port, &button_4_data);
	printk("Set up button 4 ok\n");
	return 0;
}


// Main program
int main(void) {

	init_led();
	init_button();

	while (true) {
		printk("Hello from main\n");
		k_msleep(1000);
		// k_yield();
	}

}


// Task to handle red led
void red_led_task(void *, void *, void*) {
	
	printk("Red led thread started\n");
	while (true) {
		// 1. set led on 
		if(led_state == 0) {

		gpio_pin_set_dt(&red,1);
		printk("Red on\n");
		// 2. sleep for 2 seconds
		k_sleep(K_SECONDS(1));
		
		// 3. set led off
		gpio_pin_set_dt(&red,0);
		printk("Red off\n");
		// 4. sleep for 2 seconds
		k_sleep(K_SECONDS(1));

		if(led_state == 0){
		led_state = 1;
		}
 
		} else {
			k_msleep(100);
		}
	}
}

// Task to handle blue led
void blue_led_task(void *, void *, void*) {
	
	printk("Blue led thread started\n");
	while (true) {
		// 1. set led on 
		gpio_pin_set_dt(&blue,1);
		printk("Blue on\n");
		// 2. sleep for 2 seconds
		k_sleep(K_SECONDS(1));
		// 3. set led off
		gpio_pin_set_dt(&blue,0);
		printk("Blue off\n");
		// 4. sleep for 2 seconds
		k_sleep(K_SECONDS(1));
	}
}

// Task to handle green led
void green_led_task(void *, void *, void*) {
	
	printk("Green led thread started\n");
	while (true) {
		if(led_state == 2) {
		// 1. set led on 
		gpio_pin_set_dt(&green,1);
		printk("Green on\n");
		// 2. sleep for 2 seconds
		k_sleep(K_SECONDS(1));
		
		// 3. set led off
		gpio_pin_set_dt(&green,0);
		printk("Green off\n");
		// 4. sleep for 2 seconds
		k_sleep(K_SECONDS(1));
		
		if(led_state == 2){
		led_state = 0;
		}
		
		} else {
			k_msleep(100);
		}
	}
}

// Task to handle yellow led
void yellow_led_task(void *, void *, void*) {
	
	printk("Yellow led thread started\n");
	while (true) {
		if(led_state == 1) {

		// 1. set led on 
		gpio_pin_set_dt(&green,1);
		gpio_pin_set_dt(&red,1);
		printk("Yellow on\n");
		// 2. sleep for 2 seconds
		k_sleep(K_SECONDS(1));
		
		// 3. set led off
		gpio_pin_set_dt(&green,0);
		gpio_pin_set_dt(&red,0);
		printk("Yellow off\n");
		// 4. sleep for 2 seconds
		k_sleep(K_SECONDS(1));
		
		if(led_state == 1){
		led_state = 2;
		}
	}
		 else {
				k_msleep(100);
			}
		
	}
}

// Task to handle yellow blinky
void yellow_blinky_task(void *, void *, void*) {
	
	printk("Yellow blinky thread started\n");
	while (true) {
		if(yellow_blinky_state == 1) {
		
		gpio_pin_set_dt(&green,1);
		gpio_pin_set_dt(&red,1);
		k_msleep(100);
		gpio_pin_set_dt(&green,0);
		gpio_pin_set_dt(&red,0);
		k_msleep(100);
			
	}
		 else {
				k_msleep(100);
			}
		
	}
}

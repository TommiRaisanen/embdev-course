#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/timing/timing.h>


/* Sulariohjelmoinnin koodaustehtävät nro. 5 / Tommi Räisänen TVT24SPL
* Yksikkötestaus
* Tähtään täysiin pisteisiin mutta katsotaan mihin päästään
* Lisään alle '*' merkin sitä mukaan kun saan tehtäviä omasta mielestä tehtävänannon mukaisesti valmiiksi :-)
*  1p suoritus: Yksinkertaiset testit ja parseri sulautetussa ohjelmassa             []
* +1p suoritus: Lisätään testikeissejä								                 []
* +1p suoritus: Lisää testausta liikennevaloihin                 					 []
* +1p suoritus: Oma lisäominaisuus    							                     []
*
*/

/****************************
 * Remember to add line:
 * CONFIG_HEAP_MEM_POOL_SIZE=1024
 * to prj.conf
 ****************************/

// Thread initializations
#define STACKSIZE 1024
#define PRIORITY 5
// Led pin configurations
static const struct gpio_dt_spec red = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);


// UART initialization
#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)
static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

// Create dispatcher FIFO buffer
K_FIFO_DEFINE(dispatcher_fifo);

// Create debug FIFO buffer
K_FIFO_DEFINE(data_fifo);


void red_led_task(void *, void *, void*);
void green_led_task(void *, void *, void*);
void yellow_led_task(void *, void *, void*);
void debug_task(void *, void *, void*);

// Condition Variables
K_MUTEX_DEFINE(red_mutex);
K_CONDVAR_DEFINE(red_signal);
K_MUTEX_DEFINE(green_mutex);
K_CONDVAR_DEFINE(green_signal);
K_MUTEX_DEFINE(yellow_mutex);
K_CONDVAR_DEFINE(yellow_signal);

K_MUTEX_DEFINE(release_mutex);
K_CONDVAR_DEFINE(release_signal);

K_THREAD_STACK_DEFINE(red_stack_area, STACKSIZE);
K_THREAD_STACK_DEFINE(yellow_stack_area, STACKSIZE);
K_THREAD_STACK_DEFINE(green_stack_area, STACKSIZE);

struct k_thread red_thread_data; 
struct k_thread yellow_thread_data; 
struct k_thread green_thread_data; 

// FIFO dispatcher data type
struct data_t {
	/*************************
	// Add fifo_reserved below
	*************************/
	void *fifo_reserved;
	char msg[20];
        uint64_t time;
};

static uint64_t sequence_total = 0;

/********************
 * init UART
 */
int init_uart(void) {
	// UART initialization
	if (!device_is_ready(uart_dev)) {
		return 1;
	} 
	return 0;
}

int init_led() {

	// Led pin initialization

        __ASSERT(device_is_ready(red.port), "Red LED GPIO device not ready");
	__ASSERT(device_is_ready(green.port), "Green LED GPIO device not ready");

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
	// set led off
	gpio_pin_set_dt(&red,   0);
	gpio_pin_set_dt(&green, 0);

	printk("Leds initialized ok\n");
	
	return 0;
}



/********************
 * Main task
 */
int main(void)
{
        timing_init();
        timing_start();
        timing_t start_time = timing_counter_get();

	init_uart();
        init_led();

	k_msleep(100);
        //printk("Program started\n");

        timing_t end_time = timing_counter_get();
        timing_stop();
        uint64_t time_us = (timing_cycles_to_ns(timing_cycles_get(&start_time,&end_time))) / 1000;
        printk("Initialization took %lld µs\n", time_us);

	while(true) {
                k_msleep(100);
        }

        return 0;
}

/********************
 * UART task
 */
static void uart_task(void *unused1, void *unused2, void *unused3)
{
	// Received character from UART
	char rc=0;
	// Message from UART
	char uart_msg[20];
	memset(uart_msg,0,20);
	int uart_msg_cnt = 0;

	while (true) {
		// Ask UART if data available
		if (uart_poll_in(uart_dev,&rc) == 0) {
			// printk("Received: %c\n",rc);
			// If character is not newline, add to UART message buffer
			if (rc != '\r') {
	                        __ASSERT(uart_msg_cnt < 20, "UART message buffer overflow");
				uart_msg[uart_msg_cnt] = rc;
				uart_msg_cnt++;
			// Character is newline, copy dispatcher data and put to FIFO buffer
			} else {
				//printk("UART msg: %s\n", uart_msg);

                                timing_start();
                                timing_t fifo_start_time = timing_counter_get();
                                
                                // FIFO Stuff begins
				
                                struct data_t *buf = k_malloc(sizeof(struct data_t));
				if (buf == NULL) {
					return;
				}
				// Copy UART message to dispatcher data
				// strncpy(buf->msg, 20, uart_msg); // mitä ihmettä, miksi kaatuu!!
				snprintf(buf->msg, 20, "%s", uart_msg);

				// You need to:
				// Put dispatcher data to FIFO buffer
                                k_fifo_put(&dispatcher_fifo, buf);

                                timing_t fifo_end_time = timing_counter_get();
                                timing_stop();
                                uint64_t fifo_time_us = (timing_cycles_to_ns(timing_cycles_get(&fifo_start_time,&fifo_end_time))) / 1000;
                                printk("UART fifo put time (µs): %lld\n", fifo_time_us);

				// Clear UART receive buffer
				uart_msg_cnt = 0;
				memset(uart_msg,0,20);

				// Clear UART message buffer
				uart_msg_cnt = 0;
				memset(uart_msg,0,20);
			}
		}
		k_msleep(10);
                //return 0;
	}
	
}

/********************
 * Dispatcher task
 */
static void dispatcher_task(void *unused1, void *unused2, void *unused3)
{
	while (true) {
		// Receive dispatcher data from uart_task fifo
		struct data_t *rec_item = k_fifo_get(&dispatcher_fifo, K_FOREVER);
		char sequence[20];
		memcpy(sequence,rec_item->msg,20);
		k_free(rec_item);

                __ASSERT(strlen(sequence) > 0, "Empty sequence received from FIFO");
                __ASSERT(strlen(sequence) < 20, "Sequence too long, buffer overflow risk");

		//printk("Dispatcher: %s\n", sequence);
                int cnt = 0;
                sequence_total = 0;

                timing_start();
                timing_t seq_start_time = timing_counter_get();

                for(cnt = 0; cnt < strlen(sequence); cnt++) {
                        char color = sequence[cnt];
                        
                        if(color == 'R' || color == 'r') {
                                
                                //printk("RED ");

                                k_thread_create(&red_thread_data, red_stack_area, STACKSIZE,
                                red_led_task, NULL, NULL, NULL,
                                PRIORITY, 0, K_NO_WAIT);
                        }
                        
                        if(color == 'Y' || color == 'y') {
                                //printk("YELLOW ");

                                k_thread_create(&yellow_thread_data, yellow_stack_area, STACKSIZE,
                                yellow_led_task, NULL, NULL, NULL,
                                PRIORITY, 0, K_NO_WAIT);
                        }
                        
                        if(color == 'G' || color == 'g') {
                                //printk("GREEN ");

                                k_thread_create(&green_thread_data, green_stack_area, STACKSIZE,
                                green_led_task, NULL, NULL, NULL,
                                PRIORITY, 0, K_NO_WAIT);
                        }
                        k_mutex_lock(&release_mutex, K_FOREVER);
                        k_condvar_wait(&release_signal, &release_mutex, K_FOREVER);
                        k_mutex_unlock(&release_mutex);
                        
                        
                }

                timing_t seq_end_time = timing_counter_get();
                timing_stop();
                uint64_t seq_process_time_us = (timing_cycles_to_ns(timing_cycles_get(&seq_start_time,&seq_end_time))) / 1000;

                struct data_t *total_buf = k_malloc(sizeof(struct data_t));
	        if (total_buf == NULL) {
			return;
                }
	total_buf->time = sequence_total;
	k_fifo_put(&data_fifo, total_buf);
	// printk("Red added to fifo: %lld\n",total_buf->time);

	k_yield();
        printk("Sequence total time (µs): %lld\n", sequence_total);
        printk("Sequence processing time in dispatcher (µs): %lld\n", seq_process_time_us);

	}
}

// Task to handle red led
void red_led_task(void *, void *, void*) {
	
        //printk("Red led thread started\n");
        timing_start();
        timing_t start_time = timing_counter_get();

		// 1. set led on 
	gpio_pin_set_dt(&red,1);
        //printk("Red on\n");
	// 2. sleep for 2 seconds
	k_sleep(K_SECONDS(1));
		
        // 3. set led off
	gpio_pin_set_dt(&red,0);
	//printk("Red off\n");
	// 4. sleep for 2 seconds
	k_sleep(K_SECONDS(1));

        

        timing_t end_time = timing_counter_get();
        timing_stop();
        
        uint64_t time_us = (timing_cycles_to_ns(timing_cycles_get(&start_time,&end_time))) / 1000;
        //printk("red_led_task took %lld µs\n", time_us);

        struct data_t *buf = k_malloc(sizeof(struct data_t));
	        if (buf == NULL) {
			return;
                }
	buf->time = time_us;
	k_fifo_put(&data_fifo, buf);
	// printk("Red added to fifo: %lld\n",buf->time);

	k_yield();

        sequence_total += time_us;

        k_condvar_broadcast(&release_signal);
        
}


// Task to handle green led
void green_led_task(void *, void *, void*) {
	
        timing_start();
        timing_t start_time = timing_counter_get();

	//printk("Green led thread started\n");
		// 1. set led on 
        gpio_pin_set_dt(&green,1);
	//printk("Green on\n");
	// 2. sleep for 2 seconds
	k_sleep(K_SECONDS(1));
		
	// 3. set led off
	gpio_pin_set_dt(&green,0);
	//printk("Green off\n");
	// 4. sleep for 2 seconds
	k_sleep(K_SECONDS(1));
        
        timing_t end_time = timing_counter_get();
        timing_stop();
        uint64_t time_us = (timing_cycles_to_ns(timing_cycles_get(&start_time,&end_time))) / 1000;
        //printk("green_led_task took %lld µs\n", time_us);

        struct data_t *buf = k_malloc(sizeof(struct data_t));
	        if (buf == NULL) {
			return;
                }
	buf->time = time_us;
	k_fifo_put(&data_fifo, buf);
	// printk("Red added to fifo: %lld\n",buf->time);

	k_yield();


        sequence_total += time_us;

        k_condvar_broadcast(&release_signal);
}

// Task to handle yellow led
void yellow_led_task(void *, void *, void*) {
	
	//printk("Yellow led thread started\n");
        timing_start();
        timing_t start_time = timing_counter_get();
		// 1. set led on 
	gpio_pin_set_dt(&green,1);
	gpio_pin_set_dt(&red,1);
	//printk("Yellow on\n");
	// 2. sleep for 2 seconds
	k_sleep(K_SECONDS(1));
		
	// 3. set led off
	gpio_pin_set_dt(&green,0);
        gpio_pin_set_dt(&red,0);
	//printk("Yellow off\n");
        k_sleep(K_SECONDS(1));

        
        timing_t end_time = timing_counter_get();
        timing_stop();

        uint64_t time_us = (timing_cycles_to_ns(timing_cycles_get(&start_time,&end_time))) / 1000;
        //printk("green_led_task took %lld µs\n", time_us);

        struct data_t *buf = k_malloc(sizeof(struct data_t));
	        if (buf == NULL) {
			return;
                }
	buf->time = time_us;
	k_fifo_put(&data_fifo, buf);
	// printk("Red added to fifo: %lld\n",buf->time);

	k_yield();

        
        sequence_total += time_us;

        k_condvar_broadcast(&release_signal);        
}

void debug_task(void *, void *, void*) {

	// Store received data
	struct data_t *received;

	while (true) {

		received = k_fifo_get(&data_fifo, K_FOREVER);
		printk("Debug received: %lld\n", received->time);
		k_free(received);

		k_yield();
	}
}

K_THREAD_DEFINE(dis_thread,STACKSIZE,dispatcher_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(uart_thread,STACKSIZE,uart_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(debug_thread,STACKSIZE,debug_task,NULL,NULL,NULL,PRIORITY,0,0);
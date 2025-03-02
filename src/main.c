/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
#include <asf.h>
#include "wifi.h"
#include "conf_board.h"
#include "conf_clock.h"
#include "ov2640.h"
#include "camera.h"
#include "timer_interface.h"

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */
	sysclk_init();
	wdt_disable(WDT);
	board_init();
	
	// Configuring timer
	configure_tc(); // configure timer
	
	// Configuring USART
	configure_usart_wifi();
	configure_wifi_comm_pin();
	
	// Configuring SPI
	configure_spi();
	spi_peripheral_initialize();
	
	
	//usart_write_line(WIFI_USART, "set spi_baud 100000\r\n"); // baud rate lower than usual to see on analyzer
	
	// INITIALIZE INDICATORS (LEDs)
	write_wifi_command("set wlan_gpio 27", 3); // YELLOW
	
	write_wifi_command("set websocket_gpio 26", 3); // WHAT PIN NUM? // GREEN
	
	write_wifi_command("set ap_gpio 25", 3); // WHAT PIN NUM? // RED
	
	// Configure Command Complete
	write_wifi_command("set comm_gpio PIN NUM", 3); // WHAT PIN NUM?
	
	// Configure network
	write_wifi_command("set net_GPIO PIN NUM", 3);
	
	// configure clients
	write_wifi_command("set clients_gpio PIN NUM", 3);
	
	// Initialize and configure camera
	init_camera();
	configure_camera();
	
	// RESET WIFI?
	// Arbitrarily hooking PA22 to EN (ESP32 is enabled when high. To reset pull low then back high)
	ioport_set_pin_dir(PIO_PA22_IDX, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(PIO_PA22_IDX, false); // resets ESP32
	ioport_set_pin_level(PIO_PA22_IDX, true); // re-enables ESP32
	
	
	
	
	// Sending "test" to WiFi and waiting for SUCCESS
	write_wifi_command("test", 3);
	counts = 0;
	while (counts < 10) {
		if (uart_test_success) {
			break;
		}
		// wait
	}
	
	

	/* Insert application code here, after the board has been initialized. */
	while(1){
		prepare_spi_transfer();
		// usart_write_line(WIFI_USART, "test\r\n");
		usart_write_line(WIFI_USART, "image_transfer 100\r\n"); // Tells ESP to ask for 100 bytes
		delay_ms(1000);
	}
}

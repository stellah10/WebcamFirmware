/*
 * wifi.c
 *
 * Created: 2/18/2025 8:29:35 AM
 *  Author: maris
 */ 

#include <asf.h>

#include "wifi.h"
#include "timer_interface.h"
#include "camera.h"

// USART Variables
volatile uint32_t received_byte_wifi = 0;
volatile bool new_rx_wifi = false;
volatile unsigned int input_pos_wifi = 0;
volatile bool wifi_comm_success = false;
volatile char input_line_wifi[MAX_INPUT_WIFI];
volatile bool uart_test_success = false;


// PROVISIONING Variables
volatile uint32_t g_ul_provision_flag = false;


// SPI Variables
volatile uint32_t transfer_index = 0;
volatile uint32_t transfer_len = 0;

// OTHER VARIABLES
// volatile uint8_t COUNTS = 0;

void WIFI_USART_HANDLER(void){
	uint32_t ul_status;
	
	/* Read USART status. */
	ul_status = usart_get_status(WIFI_USART);
	
	/* Read USART status */
	if (ul_status & US_CSR_RXBUFF) {
		usart_read(WIFI_USART, &received_byte_wifi);
		new_rx_wifi = true;
		process_incoming_byte_wifi((uint8_t)received_byte_wifi);
	}
}

static void wifi_command_response_handler(uint32_t ul_id, uint32_t ul_mask)
{
	unused(ul_id);
	unused(ul_mask);

	wifi_comm_success = true;
	process_data_wifi();
	for (int jj=0;jj<MAX_INPUT_WIFI;jj++) input_line_wifi[jj] = 0;
	input_pos_wifi = 0;
}


void process_incoming_byte_wifi(uint8_t in_byte)
{
	input_line_wifi[input_pos_wifi++] = in_byte;
}

void process_data_wifi(void)
{
	if (strstr(input_line_wifi, "SUCCESS")) {
		uart_test_success = true; // REPLACE WITH ACTUAL CODE FOR CAMERA
	}
}

void configure_usart_wifi(void)
{
	gpio_configure_pin(PIN_USART0_RXD_IDX, PIN_USART0_RXD_FLAGS);
	gpio_configure_pin(PIN_USART0_TXD_IDX, PIN_USART0_TXD_FLAGS);
	
	const sam_usart_opt_t usart_console_settings = {
		WIFI_USART_BAUDRATE,
		US_MR_CHRL_8_BIT,
		US_MR_PAR_NO,
		US_MR_NBSTOP_1_BIT,
		US_MR_CHMODE_NORMAL,
		/* This field is only used in IrDA mode. */
		0
	};
	
	/* Enable the peripheral clock */
	sysclk_enable_peripheral_clock(WIFI_USART_ID);
	
	/* Configure USART */
	usart_init_rs232(WIFI_USART, &usart_console_settings,sysclk_get_peripheral_hz());
	
	/* Disable all the interrupts. */
	usart_disable_interrupt(WIFI_USART, ALL_INTERRUPT_MASK);
	
	/* Enable the receiver and transmitter. */
	usart_enable_tx(WIFI_USART);
	usart_enable_rx(WIFI_USART);

	/* Configure and enable interrupt of USART. */
	NVIC_EnableIRQ(WIFI_USART_IRQn);
	
	usart_enable_interrupt(WIFI_USART, US_IER_RXRDY);
}

void configure_wifi_comm_pin(void)
{
	/* Configure PIO clock */
	pmc_enable_periph_clk(WIFI_COMM_ID);
	
	/* Initialize PIO interrupt handler */
	pio_handler_set(WIFI_COMM_PIO, WIFI_COMM_ID, WIFI_COMM_PIN_NUM, WIFI_COMM_ATTR, wifi_command_response_handler);
	
	/* Enable PIO controller IRQs. */
	NVIC_EnableIRQ((IRQn_Type)WIFI_COMM_ID);

	/* Enable PIO interrupt lines. */
	pio_enable_interrupt(WIFI_COMM_PIO, WIFI_COMM_PIN_NUM);

}

void configure_spi(void)
{
	gpio_configure_pin(SPI_MISO_GPIO, SPI_MISO_FLAGS);
	gpio_configure_pin(SPI_MOSI_GPIO, SPI_MOSI_FLAGS);
	gpio_configure_pin(SPI_SPCK_GPIO, SPI_SPCK_FLAGS);
	gpio_configure_pin(SPI_NPCS0_GPIO, SPI_NPCS0_FLAGS);
	
	/* Configure SPI interrupts for slave only. */
	NVIC_DisableIRQ(SPI_IRQn);
	NVIC_ClearPendingIRQ(SPI_IRQn);
	NVIC_SetPriority(SPI_IRQn, 0);
	NVIC_EnableIRQ(SPI_IRQn);
}

void spi_peripheral_initialize(void)
{
	spi_enable_clock(SPI);
	spi_disable(SPI);
	spi_reset(SPI);
	spi_set_slave_mode(SPI);
	spi_disable_mode_fault_detect(SPI);
	spi_set_peripheral_chip_select_value(SPI, SPI_CHIP_PCS);
	spi_set_clock_polarity(SPI, SPI_CHIP_SEL, SPI_CLK_POLARITY);
	spi_set_clock_phase(SPI, SPI_CHIP_SEL, SPI_CLK_PHASE);
	spi_set_bits_per_transfer(SPI, SPI_CHIP_SEL, SPI_CSR_BITS_8_BIT);
	spi_enable_interrupt(SPI, SPI_IER_RDRF);
	spi_enable(SPI);
}

void prepare_spi_transfer(void)
{
	transfer_len = image_length; // CORRECT?
	transfer_index = 0;
}

void SPI_Handler(void) // wifi_spi_handler
{
	uint32_t new_cmd = 0;
	static uint16_t data;
	uint8_t uc_pcs;

	if (spi_read_status(SPI) & SPI_SR_RDRF) {
		spi_read(SPI, &data, &uc_pcs);
		
		if (transfer_len--) {
			spi_write(SPI, transfer_index++, 0, 0);
		}
	}
}

void wifi_provision_handler(uint32_t ul_id, uint32_t ul_mask)
{
	// take code from button hw
	unused(ul_id);
	unused(ul_mask);
	
	g_ul_provision_flag = true;
}

void configure_wifi_provision_pin(void)
{
	// Configuring button
	/* Configure PIO clock. */
	pmc_enable_periph_clk(PROVISION_BUTTON_ID);

	/* Adjust PIO debounce filter using a 10 Hz filter. */
	pio_set_debounce_filter(PROVISION_BUTTON_PIO, PROVISION_BUTTON_MASK, 10);

	/* Initialize PIO interrupt handler, see PIO definition in conf_board.h
	**/
	pio_handler_set(PROVISION_BUTTON_PIO, PROVISION_BUTTON_ID, PROVISION_BUTTON_MASK,
			PROVISION_BUTTON_ATTR, wifi_provision_handler);

	/* Enable PIO controller IRQs. */
	NVIC_EnableIRQ((IRQn_Type)PROVISION_BUTTON_ID);

	/* Enable PIO interrupt lines. */
	pio_enable_interrupt(PROVISION_BUTTON_PIO, PROVISION_BUTTON_MASK);
	
}

void write_wifi_command(char* comm, uint8_t cnt)
{
	counts = 0;
	
	usart_write_line(WIFI_USART, comm); // Tells ESP "comm" command

	// Wait for either command completion or timeout
	while (counts < cnt) {
		// check if command complete pin is high
		if (wifi_comm_success) {
			return; // Exit function if acknowledgment is received
		}
	}
	// move on
}

void write_image_to_web(void)
{
	if (image_length == 0) {
		return;
	}
	else {
		prepare_spi_transfer(); // set necessary parameters to prepare for SPI transfer
		
		// Issue command "image_transfer xxx", where xxxx is the length of the image to be transferred
		char command[50];  // Ensure the buffer is large enough
		sprintf(command, "image_transfer %d", image_length);
		write_wifi_command(command, 3);
		//write_wifi_command("image_transfer " + sprintf(image_length), 3); // WHAT TO SET FOR CNT?
		
		while (wifi_comm_success == 0) { // ESP32 sets "command complete" pin low and begins transferring the image over SPI
			// wait
		}
		
	}
	return;
}
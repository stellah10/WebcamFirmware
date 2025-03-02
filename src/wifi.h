/*
 * wifi.h
 *
 * Created: 2/18/2025 8:29:55 AM
 *  Author: maris
 */ 



#ifndef WIFI_H_
#define WIFI_H_

#include <asf.h>
#include <string.h>


/** All interrupt mask. */
#define ALL_INTERRUPT_MASK  0xffffffff

// WiFi control pin definitions
#define WIFI_ControlPin1 PIO_PB3_IDX
#define WIFI_ControlPin2 PIO_PB2_IDX
#define WIFI_ControlPin3 PIO_PB1_IDX
#define WIFI_ControlPin4 PIO_PB0_IDX

// WiFI UART parameters and pin definitions
#define WIFI_USART_ID			ID_USART0
#define WIFI_USART				USART0
#define WIFI_USART_BAUDRATE		115200
#define WIFI_USART_HANDLER		USART0_Handler
#define WIFI_USART_IRQn				USART0_IRQn
#define WIFI_USART_CHAR_LENGTH	US_MR_CHRL_8_BIT
#define WIFI_USART_PARITY		US_MR_PAR_NO
#define WIFI_USART_STOP_BITS	US_MR_NBSTOP_1_BIT
#define WIFI_USART_MODE			US_MR_CHMODE_NORMAL

// USART0 RX pin
#define PIN_USART0_RXD_IDX		(PIO_PA5_IDX)
#define PIN_USART0_RXD_FLAGS	(PIO_PERIPH_A | PIO_PULLUP)
// USART0 TX pin
#define PIN_USART0_TXD_IDX		(PIO_PA6_IDX)
#define PIN_USART0_TXD_FLAGS	(PIO_PERIPH_A | PIO_PULLUP)

#define WIFI_COMM_PIN_NUM		PIO_PB10
#define WIFI_COMM_PIO			PIOB
#define WIFI_COMM_ID			ID_PIOB
#define WIFI_COMM_ATTR			PIO_IT_RISE_EDGE

#define MAX_INPUT_WIFI			1000
volatile char input_line_wifi[MAX_INPUT_WIFI];
volatile uint32_t received_byte_wifi;
volatile bool new_rx_wifi;
volatile unsigned int input_pos_wifi; // used in process_incoming_byte_wifi
volatile bool wifi_comm_success;
volatile uint32_t g_ul_provision_flag;
volatile bool uart_test_success;


void configure_usart_wifi(void);
void process_incoming_byte_wifi(uint8_t inByte);
void process_data_wifi(void);
void configure_wifi_comm_pin(void);

// WiFI SPI parameters and pin definitions
/** SPI MISO pin definition. */
#define SPI_MISO_GPIO			(PIO_PA12_IDX)
#define SPI_MISO_FLAGS			(PIO_PERIPH_A | PIO_DEFAULT)
/** SPI MOSI pin definition. */
#define SPI_MOSI_GPIO			(PIO_PA13_IDX)
#define SPI_MOSI_FLAGS			(PIO_PERIPH_A | PIO_DEFAULT)
/** SPI SPCK pin definition. */
#define SPI_SPCK_GPIO			(PIO_PA14_IDX)
#define SPI_SPCK_FLAGS			(PIO_PERIPH_A | PIO_DEFAULT)

/** SPI chip select 0 pin definition. (Only one configuration is possible) */
#define SPI_NPCS0_GPIO			(PIO_PA11_IDX)
#define SPI_NPCS0_FLAGS         (PIO_PERIPH_A | PIO_DEFAULT)

/* Chip select. */
#define SPI_CHIP_SEL 0
#define SPI_CHIP_PCS spi_get_pcs(SPI_CHIP_SEL)

/* Clock polarity. */
#define SPI_CLK_POLARITY 0

/* Clock phase. */
#define SPI_CLK_PHASE 0//1

volatile uint32_t transfer_index;
volatile uint32_t transfer_len;


void configure_spi(void);
void prepare_spi_transfer(void);
void spi_peripheral_initialize(void);

// PROVISIONING
void wifi_provision_handler(uint32_t ul_id, uint32_t ul_mask);
void configure_wifi_provision_pin(void);

#define PROVISION_BUTTON               {PIO_PA18, PIOA, ID_PIOA, PIO_INPUT, \
										PIO_PULLUP | PIO_DEBOUNCE | \
										PIO_IT_RISE_EDGE}
#define PROVISION_BUTTON_MASK          PIO_PA18
#define PROVISION_BUTTON_PIO           PIOA
#define PROVISION_BUTTON_ID            ID_PIOA
#define PROVISION_BUTTON_TYPE          PIO_INPUT
#define PROVISION_BUTTON_ATTR          PIO_PULLUP | PIO_DEBOUNCE | \
										PIO_IT_RISE_EDGE

#define GPIO_PROVISION_BUTTON            (PIO_PA18_IDX)
#define GPIO_PROVISION_BUTTON_FLAGS       (PIO_INPUT | PIO_PULLUP | PIO_DEBOUNCE | \
										PIO_IT_RISE_EDGE)
										
// OTHER FUNCTIONS
// volatile uint8_t COUNTS;
void write_wifi_command(char* comm, uint8_t cnt);
void write_image_to_web(void);

#endif /* WIFI_H_ */
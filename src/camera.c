/*
 * camera.c
 *
 * Created: 2/18/2025 8:30:08 AM
 *  Author: maris
 */ 

#include "camera.h"
#include "ov2640.h"

volatile uint32_t g_ul_vsync_flag = false;
volatile uint8_t CAP_DEST[10000]; // destination to store captured image
volatile uint8_t SOI = NULL; // index of start of image
volatile uint8_t EOI = NULL; // index of end of image
volatile uint8_t image_length = 0;


/* Pointer to the image data destination buffer */
uint8_t *g_p_uc_cap_dest_buf;

/* Handler for vertical synchronization using by the OV2640 image sensor.*/
void vsync_handler(uint32_t ul_id, uint32_t ul_mask)
{
	unused(ul_id);
	unused(ul_mask);

	g_ul_vsync_flag = true;
}

/* Initialize Vsync_Handler*/
void init_vsync_interrupts(void)
{
	/* Initialize PIO interrupt handler, see PIO definition in conf_board.h
	**/
	pio_handler_set(OV_VSYNC_PIO, OV_VSYNC_ID, OV_VSYNC_MASK,
			OV_VSYNC_TYPE, vsync_handler);

	/* Enable PIO controller IRQs */
	NVIC_EnableIRQ((IRQn_Type)OV_VSYNC_ID);
}

void configure_twi(void)
{
	twi_options_t opt;
	/* Enable TWI peripheral */
	pmc_enable_periph_clk(ID_BOARD_TWI);

	/* Init TWI peripheral */
	opt.master_clk = sysclk_get_cpu_hz();
	opt.speed      = TWI_CLK;
	twi_master_init(BOARD_TWI, &opt);
	
	/* Configure TWI interrupts */
	NVIC_DisableIRQ(BOARD_TWI_IRQn);
	NVIC_ClearPendingIRQ(BOARD_TWI_IRQn);
	NVIC_SetPriority(BOARD_TWI_IRQn, 0);
	NVIC_EnableIRQ(BOARD_TWI_IRQn);
	
	/* ov2640 Initialization */
	while (ov_init(BOARD_TWI) == 1) {
	}
	
}

/**
 * \brief Initialize PIO capture for the OV2640 image sensor communication.
 *
 * \param p_pio PIO instance to be configured in PIO capture mode.
 * \param ul_id Corresponding PIO ID.
 */
void pio_capture_init(Pio *p_pio, uint32_t ul_id)
{
	/* Enable periphral clock */
	pmc_enable_periph_clk(ul_id);

	/* Disable pio capture */
	p_pio->PIO_PCMR &= ~((uint32_t)PIO_PCMR_PCEN);

	/* Disable rxbuff interrupt */
	p_pio->PIO_PCIDR |= PIO_PCIDR_RXBUFF;

	/* 32bit width*/
	p_pio->PIO_PCMR &= ~((uint32_t)PIO_PCMR_DSIZE_Msk);
	p_pio->PIO_PCMR |= PIO_PCMR_DSIZE_WORD;

	/* Only HSYNC and VSYNC enabled */
	p_pio->PIO_PCMR &= ~((uint32_t)PIO_PCMR_ALWYS);
	p_pio->PIO_PCMR &= ~((uint32_t)PIO_PCMR_HALFS);

#if !defined(DEFAULT_MODE_COLORED)
	/* Samples only data with even index */
	p_pio->PIO_PCMR |= PIO_PCMR_HALFS;
	p_pio->PIO_PCMR &= ~((uint32_t)PIO_PCMR_FRSTS);
#endif
}

uint8_t pio_capture_to_buffer(Pio *p_pio, uint8_t *uc_buf, uint32_t ul_size)
{
	/* Check if the first PDC bank is free */
	if ((p_pio->PIO_RCR == 0) && (p_pio->PIO_RNCR == 0)) {
		p_pio->PIO_RPR = (uint32_t)uc_buf;
		p_pio->PIO_RCR = ul_size;
		p_pio->PIO_PTCR = PIO_PTCR_RXTEN;
		return 1;
	} else if (p_pio->PIO_RNCR == 0) {
		p_pio->PIO_RNPR = (uint32_t)uc_buf;
		p_pio->PIO_RNCR = ul_size;
		return 1;
	} else {
		return 0;
	}
}

void init_camera(void)
{
	/* Init Vsync handler*/
	init_vsync_interrupts();

	/* Init PIO capture*/
	pio_capture_init(OV_DATA_BUS_PIO, OV_DATA_BUS_ID);

	/* Turn on ov7740 image sensor using power pin */
	uint32_t t = 1;
	ov_power(t, OV_RST_PIO, OV_RST_MASK); // Camera resets when it is pulled low
	
	/* Init PCK0 to work at 24 Mhz */
	/* 96/4=24 Mhz */
	PMC->PMC_PCK[0] = (PMC_PCK_PRES_CLK_4 | PMC_PCK_CSS_PLLA_CLK);
	PMC->PMC_SCER = PMC_SCER_PCK0;
	while (!(PMC->PMC_SCSR & PMC_SCSR_PCK0)) {
	}
	
	configure_twi();
	
}

void configure_camera(void)
{
	/* ov2640 configuration */
	ov_configure(BOARD_TWI, JPEG_INIT);
	ov_configure(BOARD_TWI, YUV422);
	ov_configure(BOARD_TWI, JPEG);
	ov_configure(BOARD_TWI, JPEG_320x240);
	
	/* Wait 3 seconds to let the image sensor to adapt to environment */
	delay_ms(3000);
}

uint8_t start_capture(void)
{
	/* Set capturing destination address*/
	// g_p_uc_cap_dest_buf = (uint8_t *)CAP_DEST;

	/* Set cap_rows value*/
	// g_us_cap_rows = IMAGE_HEIGHT; // LCD board height for OV7740

	/* Enable vsync interrupt*/
	pio_enable_interrupt(OV_VSYNC_PIO, OV_VSYNC_MASK);

	/* Capture acquisition will start on rising edge of Vsync signal.
	 * So wait g_vsync_flag = 1 before start process
	 */
	while (!g_ul_vsync_flag) {
	}

	/* Disable vsync interrupt*/
	pio_disable_interrupt(OV_VSYNC_PIO, OV_VSYNC_MASK);
	
	// VSYNC LOW?

	/* Enable pio capture*/
	pio_capture_enable(OV_DATA_BUS_PIO);


	/* Capture data and send it to external SRAM memory thanks to PDC
	 * feature */
	/*
	NOT USING SRAM
	pio_capture_to_buffer(OV_DATA_BUS_PIO, g_p_uc_cap_dest_buf,
			(g_us_cap_line * g_us_cap_rows) >> 2); */ 
	// ul_size = end - start + 1;
	pio_capture_to_buffer(OV_DATA_BUS_PIO, CAP_DEST, 10000 >> 2); // byte = 8, word = 32; ul_buf expects word
	// saves image(s) to CAP_DEST

	/* Wait end of capture*/
	while (!((OV_DATA_BUS_PIO->PIO_PCISR & PIO_PCIMR_RXBUFF) ==
			PIO_PCIMR_RXBUFF)) {
	}

	/* Disable pio capture*/
	pio_capture_disable(OV_DATA_BUS_PIO);

	/* Reset vsync flag*/
	g_ul_vsync_flag = false;
	
	// find image length
	uint8_t image_len = find_image_len();
	if (image_len == 1) // image length is nonzero
	{
		return 1;
	}
	else{
		return 0; //error
	}
}

uint8_t find_image_len(void)
{
	SOI = NULL;
	EOI = NULL;
	
	uint32_t buffer_size = sizeof(CAP_DEST);
	for (uint32_t i=0; i<buffer_size -1; i++) {
		uint16_t marker = (CAP_DEST[i] << 8) | CAP_DEST[i + 1];
		
		if (marker == 0xFFD8 && SOI == NULL){// haven't seen SOI before
			SOI = i; // first occurence of SOI
		}
		else if (marker == 0xFFD9 && SOI != NULL) { // found SOI; looking for EOI
			EOI = i+1; // first occurence of EOI after SOI
			break; // found both markers
		}
	}
	
	if (SOI != NULL && EOI != NULL && SOI < EOI) {
		image_length = (EOI - SOI) + 1;
		return 1;
	}
	
	return 0; // Error
}

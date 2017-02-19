/*
 * CANRegisters.h
 *
 *  Created on: Jan 18, 2017
 *      Author: Scott
 */

#ifndef CANREGISTERS_H_
#define CANREGISTERS_H_

#include <stddef.h>

#define CAN_REGISTER_BANK 2

#define NUM_ACCEPT_FILTERS_RX0_BUFFER 	 2
#define NUM_ACCEPT_FILTERS_RX1_BUFFER	 4

typedef struct {
	uint16_t can_2_0b		: 1; /* Support for Can Bus 2.0b standard */
	uint16_t unused			: 15;
} CAN_CAPABILITY_FLAGS;

typedef struct {
	uint8_t rx_fifo_nonempty: 1; /* Received data available in Rx FIFO    */
	uint8_t tx_fifo_empty   : 1; /* Successful transmission from Tx FIFO  */
	uint8_t bus_error   	: 1; /* Error flags/counts provide details    */
	uint8_t message_error   : 1; /* Used for Baud Rate determination
									(in LISTEN-only Mode)                 */
	uint8_t wake			: 1; /* Set in SLEEP mode if CAN bus activity
									occurs (auto-enters LISTEN-only Mode) */
	uint8_t sw_rx_overflow	: 1; /* Set if data was received when Rx FIFO
									was full.  No response necessary.
									Will be auto-cleared when Rx FIFO is
									no longer full.                       */
	uint8_t hw_rx_overflow	: 1; /* Hardware Receive overflow.  MUST BE
									CLEARED in order to receive more data.*/
	uint8_t					: 1;
} CAN_IFX_INT_FLAGS;

typedef struct {
	/* NOTE:  The following errors are auto-cleared by the CAN hardware.      */
	uint8_t	can_bus_warn	: 1; /* Rx or Tx error count > 96			  	  */
	uint8_t can_bus_err_pasv: 1; /* Rx or Tx error count > 128            	  */
	uint8_t can_bux_tx_off  : 1; /* Tx error count == 255.  Note:  Will   	  */
								 /* auto-recover.  RESET to force clear. 	  */
	uint8_t					: 5;
} CAN_ERROR_FLAGS;

typedef enum {
	CAN_MODE_NORMAL,
	CAN_MODE_SLEEP,
	CAN_MODE_LOOP,
	CAN_MODE_LISTEN,
	CAN_MODE_CONFIG,
} CAN_MODE;

typedef struct {
    uint8_t	eidt	: 2; /* Lowest 2 bits of Extended Identifier */
    uint8_t invalid : 1; /* If set, data is not valid */
    uint8_t ide     : 1; /* Extended Identifier Flag */
    uint8_t srr     : 1; /* Frame Remote Transmit Request (Standard only) */
    uint8_t sidl    : 3; /* Lowest 3 bits of Standard Identifier */
} CAN_ID_FLAGS;

typedef struct {
	uint8_t     	sidh;
    CAN_ID_FLAGS    sidl;
    uint8_t         eidh;
    uint8_t         eidl;
} CAN_ID;

typedef struct {
    uint8_t len     : 4;
    uint8_t         : 2;
    uint8_t rtr     : 1;
    uint8_t         : 1;
} CAN_DLC;

typedef struct {
	CAN_DLC        	dlc;
    uint8_t         buff[ 8 ];
} CAN_DATA;

typedef struct {
	CAN_ID			id;
	CAN_DATA		payload;
} CAN_TRANSFER;

typedef struct {
	uint8_t sam		: 1; /* True: one sample point is located between PHSEG1 and
	 	 	 	 	 	 	PHSEG2.  Additionally, two samples are taken at one-
	 	 	 	 	 	 	half TQ intervals prior to the end of PHSEG1, with
			 	 	 	 	bit valueg determined by majority decision. */
	uint8_t btlmode : 1; /* PS2 Bit Time Length bit.  True:  Length of PS2
							determined by PHSEG2<2:0> bits of CNF3; False:
							Length is greater of PS1 & IPT (2 Time Quantums) */
	uint8_t wakfil 	: 1; /* True: Wake-up filter enabled */
	uint8_t unused	: 5;
} CAN_ADV_CONFIG_OPTIONS;

struct __attribute__ ((__packed__)) CAN_ADV_CONFIG {
	/*-- Bit Timing Configuration - see MCP25625 Datasheet section 3.8. --*/
	uint8_t baud_rate_prescaler; /* (0-63) */
	uint8_t sjw; /* Synchronization Jump Width Length (0-3) */
	/* Propagation/Phase Time Quantums (range 0-7 [= Time Quantum +1]) */
	uint8_t prseg; /* Propagation Segment.  7 allows 40m bus length. */
	uint8_t phseg1;	/* Phase Segment 1 compensates for phase shift on edges. */
	uint8_t phseg2; /* Phase Segment 2 compensates for phase shift on edges. */
	/* Advanced Configuration Flags */
	CAN_ADV_CONFIG_OPTIONS adv_config_options;
};

#define MAX_TX_ENTRIES		10 /* See CANRegisters.h */
#define MAX_RX_ENTRIES		10 /* See CANRegisters.h */

/* Interrupt pin:  Active Low, held until interrupt cleared in int flags */

#define BIT_MODIFY_COMMAND_REG 0x7F

/* BitModify SPI command:
 *
 * allows contents of any single register to be modified using a bit mask.
 * Typically used for interrupt acknowledgement.
 *
 * Format: [Bank] [0x80 | 0x7F] [Count=3] [RegAddr] [Mask Byte] [DataByte] */

/* Transmit FIFO Write Command:
 *
 * Allows one CAN_DATA structure (9 bytes) to be written to the transmit fifo.
 * If the transmit fifo is full, this command is ignored.
 *
 * Format: [Bank] [0x80 | TxFifoHeadAddress] [Count=9] [9 bytes of data]
 *
 * Upon success, the tx fifo count is automatically incremented.
 *
 * This command is only treated as a Transmit FIFO Write Command if the
 * register address is the TxFifoHeadAddress; otherwise, this will be treated
 * as a burst write command.
 */

/* Transmit FIFO Read Command:
 *
 * Allows one or more CAN_TRANSFER structures (13 bytes each) to be read from
 * the receive fifo.  The receive fifo count will be automatically decremented
 * by the number of CAN_TRANSFER structures read.
 *
 * Note:  The Count of bytes to be returned must be a multiple of 13 bytes.  If
 * the count is not an integer multiple of 13, the fifo count will be
 * decremented by the number of complete CAN_TRANSFER structures read.
 *
 * If the receive fifo is empty, the "invalid" flag will be set in returned
 * CAN_TRANSFER structures, and also the dlc length and all data bytes will be
 * set to 0.
 *
 * This command is only treated as a Receive FIFO Read Command if the
 * register address is the RxFifoTailAddress; otherwise, this will be treated
 * as a burst read command.
 *
 * NOTE:  If the Receive FIFO Read Command causes the Receive FIFO count to
 * drop to 0, the "rx_fifo_nonempty" interrupt flag will automatically be
 * cleared.
 */

/* Attempts to perform writes to read-only registers will have no effect. */

/* Attempts to access the TxFifo not using the Transmit Fifo Write Command
 * will be ignored; attempts to write to the RxFifo will be ignored; attempts
 * to read the RxFifo not using the Receive Fifo Read Command return undefined
 * values.
 */

/* Burst Reads:
 *
 * Attempts to perform burst reads starting at/before either tx or rx fifo
 * address will return undefined values from those fifo addresses.
 */

/* Burst Writes:
 *
 * Attempts to perform burst writes (i.e., NOT via the Transmit FIFO Write
 * Command) that start before the TxFifoHeadAddress and end at or after that
 * address will not cause any change to the Transmit Fifo.
 */

#define RESET_COMMAND 0x5A

struct __attribute__ ((__packed__)) CAN_REGS {
	/****************************/
	/* Capabilities (Read-only) */
	/****************************/
	CAN_CAPABILITY_FLAGS capability_flags;

	/**********************/
	/* Status (Read-only) */
	/**********************/
	CAN_IFX_INT_FLAGS int_status; /* Shadow of int_flags   */
	uint8_t rx_fifo_entry_count;  /* (0 to MAX_RX_ENTRIES) */
	uint8_t tx_fifo_entry_count;  /* (0 to MAX_TX_ENTRIES) */
	CAN_ERROR_FLAGS bus_error_flags;
	uint8_t tx_err_count;
	uint8_t rx_err_count;

	/******************************/
	/* Transmit FIFO (Write-only) */
	/******************************/
	uint8_t tx_fifo_head; /* Up to MAX_TX_ENTRIES of CAN_DATA entries */
	/****************************/
	/* Receive FIFO (Read-only) */
	/****************************/
	uint8_t rx_fifo_tail; /* Up to MAX_RX_ENTRIES of CAN_TRANSFER entries */

	/*********************************************/
	/* Interrupt Status/Acknowledge (Read/Write) */
	/*********************************************/
	CAN_IFX_INT_FLAGS int_enable; /* Note:  Use BitModify command to modify. */
	CAN_IFX_INT_FLAGS int_flags;  /* Note:  Use BitModify command to modify. */

	/************************************/
	/* Basic Configuration (Read/Write) */
	/************************************/
	CAN_MODE opmode;

	/************************************/
	/* Reset (Write-only) */
	/************************************/
	uint8_t reset;	/* Set to RESET_COMMAND to force hardware reset. */

	/************************************************************************/
	/*-- Acceptance Filters/Masks - see MCP25625 Datasheet section 3.7.5. --*/
	/*                                                                      */
	/* NOTE:  Must be in CAN_MODE_CONFIG for changes to take effect!        */
	/************************************************************************/
	CAN_ID accept_mask_rxb0;
	CAN_ID accept_filter_rxb0[NUM_ACCEPT_FILTERS_RX0_BUFFER];
	CAN_ID accept_mask_rxb1;
	CAN_ID accept_filter_rxb1[NUM_ACCEPT_FILTERS_RX1_BUFFER];

	/*****************************************************************/
	/* Advanced Configuration (Read/Write)                           */
	/*                                                               */
	/* NOTE:  Must be in CAN_MODE_CONFIG for changes to take effect! */
	/*****************************************************************/
	struct CAN_ADV_CONFIG advanced_config;
	uint8_t end_of_bank;
};

inline void CAN_unpack_standard_id(CAN_ID *id, uint32_t *STANDARD_ID) {
	*STANDARD_ID = 0;
	*STANDARD_ID |= id->sidh;
	*STANDARD_ID <<= 3;
	*STANDARD_ID |= id->sidl.sidl;
}

inline void CAN_unpack_extended_id(CAN_ID *id, uint32_t *EXTENDED_ID) {
	*EXTENDED_ID = 0;
	*EXTENDED_ID |= id->sidh;
	*EXTENDED_ID <<= 3;
	*EXTENDED_ID |= id->sidl.sidl;
	*EXTENDED_ID <<= 2;
	*EXTENDED_ID |= id->sidl.eidt;
	*EXTENDED_ID <<= 8;
	*EXTENDED_ID |= id->eidh;
	*EXTENDED_ID <<= 8;
	*EXTENDED_ID |= id->eidl;
}

inline void CAN_pack_standard_id(uint32_t SID, CAN_ID *id) {
	id->sidh = (SID & 0x07F8) >> 3;
	id->sidl.sidl = (SID & 0x0007);
}

inline void CAN_pack_extended_id(uint32_t EID, CAN_ID *id) {
	id->sidh = (EID & 0x1FE00000) >> 21;
	id->sidl.sidl = (EID & 0x001C0000) >> 18;
	id->sidl.eidt = (EID & 0x00030000) >> 16;
	id->eidh = (EID & 0x0000FF00) >> 8;
	id->eidl = (EID & 0x000000FF);
}

#endif /* CANREGISTERS_H_ */
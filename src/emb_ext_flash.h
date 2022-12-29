#ifndef EMB_EXT_FLASH_H_
#define EMB_EXT_FLASH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// Generic commands, these are generic JEDEC commands for flash memory chips - not specific to any one chip.
#define EXT_FLASH_CMD_WRITE_ENABLE 0x06
#define EXT_FLASH_CMD_WRITE_DISABLE 0x04
#define EXT_FLASH_CMD_READ_STATUS_REG 0x05
#define EXT_FLASH_CMD_WRITE_STATUS_REG 0x01
#define EXT_FLASH_CMD_READ_DATA 0x03
#define EXT_FLASH_CMD_FAST_READ 0x0B
#define EXT_FLASH_CMD_PAGE_PROGRAM 0x02
#define EXT_FLASH_CMD_SECTOR_ERASE 0x20
#define EXT_FLASH_CMD_BLOCK_ERASE_32K 0x52
#define EXT_FLASH_CMD_BLOCK_ERASE_64K 0xD8
#define EXT_FLASH_CMD_CHIP_ERASE 0xC7
#define EXT_FLASH_CMD_POWER_DOWN 0xB9
#define EXT_FLASH_CMD_RELEASE_POWER_DOWN 0xAB
#define EXT_FLASH_CMD_JEDEC_ID 0x9F

// Generic status register bits
#define EXT_FLASH_STATUS_REG_BUSY 0x01
#define EXT_FLASH_STATUS_REG_WEL 0x02

/**
 * @brief emb_ext_flash_transfer exchange bytes with the external flash memory chip, this function must be implemented by the user.
 * The function is intentionally left ambiguous to support mulitple interfaces, including but not limited to SPI, UART, I2C, etc.
 *
 * @param id - the id of the external flash memory chip, this is used to support multiple chips. The ID is arbitrary and is set by the user.
 * @param tx_data - pointer to the data to be sent to the external flash memory chip.
 * @param rx_data - pointer to the data to be received from the external flash memory chip.
 * @param len - the number of bytes to be sent and received.
 * @return int - number of bytes successfully transferred.
 */
int emb_ext_flash_transfer( int id, uint8_t *tx_data, uint8_t *rx_data, uint16_t len );

/**
 * @brief emb_ext_flash_get_jedec_id get the JEDEC ID of the external flash memory chip.
 *
 * @param id - the id of the external flash memory chip, this is used to support multiple chips. The ID is arbitrary and is set by the user.
 * @param manufacturer_id - pointer to the manufacturer ID to be read.
 * @param memory_type - pointer to the memory type to be read.
 * @param capacity - pointer to the capacity to be read.
 * @return int - 0 on success, -1 on failure.
 */
int emb_ext_flash_get_jedec_id( int id, uint8_t *manufacturer_id, uint8_t *memory_type, uint8_t *capacity );

/**
 * @brief emb_ext_flash_read read data from the external flash memory chip.
 *
 * @param id - the id of the external flash memory chip, this is used to support multiple chips. The ID is arbitrary and is set by the user.
 * @param address - the address to read from.
 * @param data - pointer to the data to be read.
 * @param len - the number of bytes to be read.
 * @return int - number of bytes successfully read.
 */
int emb_ext_flash_read( int id, uint32_t address, uint8_t *data, uint16_t len );

/**
 * @brief emb_ext_flash_write write data to the external flash memory chip.
 *
 * @param id - the id of the external flash memory chip, this is used to support multiple chips. The ID is arbitrary and is set by the user.
 * @param address - the address to write to.
 * @param data - pointer to the data to be written.
 * @param len - the number of bytes to be written.
 * @return int - number of bytes successfully written.
 */
int emb_ext_flash_write( int id, uint32_t address, uint8_t *data, uint16_t len );

/**
 * @brief emb_ext_flash_erase erase the external flash memory chip.
 *
 * @param id - the id of the external flash memory chip, this is used to support multiple chips. The ID is arbitrary and is set by the user.
 * @param address - the address to erase from.
 * @param len - the number of bytes to be erased.
 * @return int - 0 on success, -1 on failure.
 */
int emb_ext_flash_erase( int id, uint32_t address, uint32_t len );

/**
 * @brief emb_ext_flash_chip_erase erase the entire external flash memory chip.
 *
 * @param id - the id of the external flash memory chip, this is used to support multiple chips. The ID is arbitrary and is set by the user.
 */
void emb_ext_flash_chip_erase( int id );

/**
 * @brief emb_ext_flash_get_status read the status register of the external flash memory chip.
 *
 * @param id - the id of the external flash memory chip, this is used to support multiple chips. The ID is arbitrary and is set by the user.
 * @return uint8_t - the status register value.
 */
uint8_t emb_ext_flash_get_status( int id );

/**
 * @brief emb_ext_flash_sleep put the external flash memory chip into sleep mode.
 *
 * @param id - the id of the external flash memory chip, this is used to support multiple chips. The ID is arbitrary and is set by the user.
 * @return int - 0 on success, -1 on failure.
 */
int emb_ext_flash_sleep( int id );

/**
 * @brief emb_ext_flash_wake wake the external flash memory chip from sleep mode.
 *
 * @param id - the id of the external flash memory chip, this is used to support multiple chips. The ID is arbitrary and is set by the user.
 * @return int - 0 on success, -1 on failure.
 */
int emb_ext_flash_wake( int id );

#ifdef __cplusplus
}
#endif

#endif /* EMB_EXT_FLASH_H_ */

/*********************************************************************************
DISCLAIMER:

This code is protected under the MIT open source license. The code is provided
"as is" without warranty of any kind, either express or implied, including but
not limited to the implied warranties of merchantability, fitness for a particular
purpose, or non-infringement. In no event shall the author or any other party be
liable for any direct, indirect, incidental, special, exemplary, or consequential
damages, however caused and on any theory of liability, whether in contract,
strict liability, or tort (including negligence or otherwise), arising in any way
out of the use of this code or performance or use of the results of this code. By
using this code, you agree to hold the author and any other party harmless from
any and all liability and to use the code at your own risk.

This code was written by GitHub user: budgettsfrog
Contact: budgettsfrog@protonmail.com
GitHub: https://github.com/warrenwoolseyiii
*********************************************************************************/

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
 * @brief emb_flash_intf_handle_t - structure to hold the interface functions for the external flash memory chip.
 * This structure is used to hold the function pointers to the interface functions for the external flash memory chip.
 * The functions are intentionally left ambiguous to support mulitple interfaces, including but not limited to SPI, UART, I2C, etc.
 */
typedef struct
{
    // Flag to indicate if the interface handle has been initialized.
    uint8_t initialized;
    // Function pointer to select the external flash memory chip.
    void ( *select )();
    // Function point to deselec the external flash memory chip.
    void ( *deselect )();
    // Function pointer to write bytes to the external flash memory chip, returns 0 if successful, -1 if not.
    int ( *write )( uint8_t *data, uint16_t len );
    // Function pointer to read bytes from the external flash memory chip, returns 0 if successful, -1 if not.
    int ( *read )( uint8_t *data, uint16_t len );
    // Function pointer to delay for a specified duration in microseconds.
    void ( *delay_us )( uint32_t duration );
} emb_flash_intf_handle_t;

/**
 * @brief emb_ext_flash_init_intf - initialize the interface handle, this function must be called before any other functions
 * for each interface handle. All this does is check each function pointer for a null value and returns -1 if any are null.
 * Otherwise it sets the initialized flag to 1 and returns 0.
 *
 * @param p_intf - pointer to the interface handle.
 * @return int - 0 on success, -1 on failure.
 */
int emb_ext_flash_init_intf( emb_flash_intf_handle_t *p_intf );

/**
 * @brief emb_ext_flash_get_jedec_id get the JEDEC ID of the external flash memory chip.
 *
 * @param p_intf - pointer to the interface handle.
 * @param manufacturer_id - pointer to the manufacturer ID to be read.
 * @param memory_type - pointer to the memory type to be read.
 * @param capacity - pointer to the capacity to be read.
 * @return int - 0 on success, -1 on failure.
 */
int emb_ext_flash_get_jedec_id( emb_flash_intf_handle_t *p_intf, uint8_t *manufacturer_id, uint8_t *memory_type, uint8_t *capacity );

/**
 * @brief emb_ext_flash_read read data from the external flash memory chip.
 *
 * @param p_intf - pointer to the interface handle.
 * @param address - the address to read from.
 * @param data - pointer to the data to be read.
 * @param len - the number of bytes to be read.
 * @return int - number of bytes successfully read.
 */
int emb_ext_flash_read( emb_flash_intf_handle_t *p_intf, uint32_t address, uint8_t *data, uint16_t len );

/**
 * @brief emb_ext_flash_write write data to the external flash memory chip.
 *
 * @param p_intf - pointer to the interface handle.
 * @param address - the address to write to.
 * @param data - pointer to the data to be written.
 * @param len - the number of bytes to be written.
 * @return int - number of bytes successfully written.
 */
int emb_ext_flash_write( emb_flash_intf_handle_t *p_intf, uint32_t address, uint8_t *data, uint16_t len );

/**
 * @brief emb_ext_flash_erase erase the external flash memory chip.
 *
 * @param p_intf - pointer to the interface handle.
 * @param address - the address to erase from.
 * @param len - the number of bytes to be erased.
 * @return int - 0 on success, -1 on failure.
 */
int emb_ext_flash_erase( emb_flash_intf_handle_t *p_intf, uint32_t address, uint32_t len );

/**
 * @brief emb_ext_flash_chip_erase erase the entire external flash memory chip.
 *
 * @param p_intf - pointer to the interface handle.
 * @return int - 0 on success, -1 on failure.
 */
int emb_ext_flash_chip_erase( emb_flash_intf_handle_t *p_intf );

/**
 * @brief emb_ext_flash_get_status read the status register of the external flash memory chip.
 *
 * @param p_intf - pointer to the interface handle.
 * @return uint8_t - the status register value.
 */
uint8_t emb_ext_flash_get_status( emb_flash_intf_handle_t *p_intf );

/**
 * @brief emb_ext_flash_sleep put the external flash memory chip into sleep mode.
 *
 * @param p_intf - pointer to the interface handle.
 * @return int - 0 on success, -1 on failure.
 */
int emb_ext_flash_sleep( emb_flash_intf_handle_t *p_intf );

/**
 * @brief emb_ext_flash_wake wake the external flash memory chip from sleep mode.
 *
 * @param p_intf - pointer to the interface handle.
 * @return int - 0 on success, -1 on failure.
 */
int emb_ext_flash_wake( emb_flash_intf_handle_t *p_intf );

/**
 * @brief emb_ext_flash_get_lib_ver get the version of the external flash memory library.
 * @return const char* - pointer to the version string.
 */
const char *emb_ext_flash_get_lib_ver();

#ifdef __cplusplus
}
#endif

#endif /* EMB_EXT_FLASH_H_ */

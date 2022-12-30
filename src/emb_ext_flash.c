#include "emb_ext_flash.h"

// User implementation
// TODO: Add anything you need specific to your implementation here.

// User functions
int emb_ext_flash_transfer( int id, uint8_t *tx_data, uint8_t *rx_data, uint16_t len )
{
    // TODO: Implement this function.
}

void emb_ext_flash_select( int id, int sel )
{
    // TODO: Implement this function.
}

void emb_ext_flash_delay_us( uint32_t duration )
{
    // TODO: Implement this function.
}

// Private functions
uint8_t emb_ext_flash_busy( int id )
{
    return ( emb_ext_flash_get_status( id ) & EXT_FLASH_STATUS_REG_BUSY );
}

void emb_ext_flash_write_enable( int id )
{
    // Make the command
    uint8_t cmd = EXT_FLASH_CMD_WRITE_ENABLE;

    // Do the transfer
    emb_ext_flash_select( id, true );
    emb_ext_flash_transfer( id, &cmd, NULL, 1 );
    emb_ext_flash_select( id, false );

    // Block while the WEL bit in the status register is unset
    while( !( emb_ext_flash_get_status( id ) & EXT_FLASH_STATUS_REG_WEL ) )
        ;
}

// Pubic functions
int emb_ext_flash_get_jedec_id( int id, uint8_t *manufacturer_id, uint8_t *memory_type, uint8_t *capacity )
{
    // Build the command and the payload
    uint8_t data[4] = { EXT_FLASH_CMD_JEDEC_ID, 0, 0, 0 };

    // Do the transfer
    emb_ext_flash_select( id, true );
    int rtn = emb_ext_flash_transfer( id, data, data, 4 );
    emb_ext_flash_select( id, false );

    // Populate the fields
    *manufacturer_id = data[1];
    *memory_type = data[2];
    *capacity = data[3];

    // Return 0 for non error, -1 for error
    return rtn == 4 ? 0 : -1;
}

int emb_ext_flash_read( int id, uint32_t address, uint8_t *data, uint16_t len )
{
    // Build the command
    uint8_t cmd[4] = { EXT_FLASH_CMD_READ_DATA, ( address >> 16 ) & 0xFF, ( address >> 8 ) & 0xFF, address & 0xFF };

    // Do the transfer
    emb_ext_flash_select( id, true );
    emb_ext_flash_transfer( id, cmd, 0, sizeof( cmd ) );
    int rtn = emb_ext_flash_transfer( id, 0, data, len );
    emb_ext_flash_select( id, false );

    // Return the number of bytes read
    return rtn;
}

int emb_ext_flash_write( int id, uint32_t address, uint8_t *data, uint16_t len )
{
    // Enable writes
    emb_ext_flash_write_enable( id );

    // Build the command
    uint8_t cmd[4] = { EXT_FLASH_CMD_PAGE_PROGRAM, ( address >> 16 ) & 0xFF, ( address >> 8 ) & 0xFF, address & 0xFF };

    // Do the transfer
    emb_ext_flash_select( id, true );
    emb_ext_flash_transfer( id, cmd, 0, sizeof( cmd ) );
    int rtn = emb_ext_flash_transfer( id, 0, data, len );
    emb_ext_flash_select( id, true );

    // Block while the flash chip commits the write
    while( emb_ext_flash_busy( id ) )
        ;

    // Return the number of bytes written
    return rtn;
}

int emb_ext_flash_erase( int id, uint32_t address, uint32_t len )
{
    // Enable writes
    emb_ext_flash_write_enable( id );

    // Determine the most efficient command to use
    uint8_t type = EXT_FLASH_CMD_SECTOR_ERASE;
    if( len > 4096 )
        type = EXT_FLASH_CMD_BLOCK_ERASE_32K;
    if( len > 32768 )
        type = EXT_FLASH_CMD_BLOCK_ERASE_64K;

    // Build the command
    uint8_t cmd[4] = { type, ( address >> 16 ) & 0xFF, ( address >> 8 ) & 0xFF, address & 0xFF };

    // Do the transfer
    emb_ext_flash_select( id, true );
    int rtn = emb_ext_flash_transfer( id, cmd, NULL, 4 );
    emb_ext_flash_select( id, false );

    // Block while the erase is committed
    while( emb_ext_flash_busy( id ) )
        ;

    // Return 0 if successful -1 otherwise
    return rtn == 4 ? 0 : -1;
}

void emb_ext_flash_chip_erase( int id )
{
    // Enable writes
    emb_ext_flash_write_enable( id );

    // Build the command
    uint8_t cmd = EXT_FLASH_CMD_CHIP_ERASE;

    // Do the transfer
    emb_ext_flash_select( id, true );
    emb_ext_flash_transfer( id, &cmd, NULL, 1 );
    emb_ext_flash_select( id, false );

    // Block while the erase is committed - this can take 2 minutes + on a chip a erase
    while( emb_ext_flash_busy( id ) )
        ;
}

uint8_t emb_ext_flash_get_status( int id )
{
    // Build the command
    uint8_t cmd[] = { EXT_FLASH_CMD_READ_STATUS_REG, 0 };

    // Do the transfer
    emb_ext_flash_select( id, true );
    emb_ext_flash_transfer( id, cmd, cmd, 2 );
    emb_ext_flash_select( id, false );

    // Return the status
    return cmd[1];
}

int emb_ext_flash_sleep( int id )
{
    // Build the  command
    uint8_t cmd = EXT_FLASH_CMD_POWER_DOWN;

    // Do the transfer
    emb_ext_flash_select( id, true );
    int rtn = emb_ext_flash_transfer( id, &cmd, NULL, 1 );
    emb_ext_flash_select( id, false );

    return rtn == 1 ? 0 : -1;
}

int emb_ext_flash_wake( int id )
{
    // Build the command
    uint8_t cmd = EXT_FLASH_CMD_RELEASE_POWER_DOWN;

    // Do the transfer
    emb_ext_flash_select( id, true );
    int rtn = emb_ext_flash_transfer( id, &cmd, 0, 1 );
    emb_ext_flash_select( id, false );

    // Wait for the specified duration for wake time - this can be optimized to your use case.
    emb_ext_flash_delay_us( 3 );

    // Return 0 for success -1 otherwise.
    return rtn == 1 ? 0 : -1;
}

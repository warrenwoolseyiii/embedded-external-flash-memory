#include "emb_ext_flash.h"

// Private functions
uint8_t emb_ext_flash_busy( emb_flash_intf_handle_t *p_intf )
{
    return ( emb_ext_flash_get_status( p_intf ) & EXT_FLASH_STATUS_REG_BUSY );
}

void emb_ext_flash_write_enable( emb_flash_intf_handle_t *p_intf )
{
    // Make the command
    uint8_t cmd = EXT_FLASH_CMD_WRITE_ENABLE;

    // Null check
    if( !p_intf )
        return;

    // Do the transfer
    p_intf->select();
    p_intf->write( &cmd, 1 );
    p_intf->deselect();

    // Block while the WEL bit in the status register is unset
    while( !( emb_ext_flash_get_status( p_intf ) & EXT_FLASH_STATUS_REG_WEL ) )
        ;
}

// Pubic functions
int emb_ext_flash_get_jedec_id( emb_flash_intf_handle_t *p_intf, uint8_t *manufacturer_id, uint8_t *memory_type, uint8_t *capacity )
{
    // Build the command and the payload
    uint8_t cmd = EXT_FLASH_CMD_JEDEC_ID;
    uint8_t data[3] = { 0 };

    // Null check
    if( !p_intf )
        return -1;

    // Do the transfer
    p_intf->select();
    p_intf->write( &cmd, 1 );
    int rtn = p_intf->read( data, 3 );
    p_intf->deselect();

    // Populate the fields
    *manufacturer_id = data[0];
    *memory_type = data[1];
    *capacity = data[2];

    // Return 0 for non error, -1 for error
    return rtn;
}

int emb_ext_flash_read( emb_flash_intf_handle_t *p_intf, uint32_t address, uint8_t *data, uint16_t len )
{
    // Build the command
    uint8_t cmd[4] = { EXT_FLASH_CMD_READ_DATA, ( address >> 16 ) & 0xFF, ( address >> 8 ) & 0xFF, address & 0xFF };

    // Null check
    if( !p_intf )
        return 0;

    // Do the transfer
    p_intf->select();
    p_intf->write( cmd, sizeof( cmd ) );
    int rtn = p_intf->read( data, len );
    p_intf->deselect();

    // Return the number of bytes read
    return ( rtn == 0 ? len : 0 );
}

int emb_ext_flash_write( emb_flash_intf_handle_t *p_intf, uint32_t address, uint8_t *data, uint16_t len )
{
    // Enable writes
    emb_ext_flash_write_enable( p_intf );

    // Build the command
    uint8_t cmd[4] = { EXT_FLASH_CMD_PAGE_PROGRAM, ( address >> 16 ) & 0xFF, ( address >> 8 ) & 0xFF, address & 0xFF };

    // Null check
    if( !p_intf )
        return 0;

    // Do the transfer
    p_intf->select();
    p_intf->write( cmd, sizeof( cmd ) );
    int rtn = p_intf->write( data, len );
    p_intf->deselect();

    // Block while the flash chip commits the write
    while( emb_ext_flash_busy( p_intf ) )
        ;

    // Return the number of bytes written
    return ( rtn == 0 ? len : 0 );
}

int emb_ext_flash_erase( emb_flash_intf_handle_t *p_intf, uint32_t address, uint32_t len )
{
    // Enable writes
    emb_ext_flash_write_enable( p_intf );

    // Null check
    if( !p_intf )
        return -1;

    // Determine the most efficient command to use
    uint8_t type = EXT_FLASH_CMD_SECTOR_ERASE;
    if( len > 4096 )
        type = EXT_FLASH_CMD_BLOCK_ERASE_32K;
    if( len > 32768 )
        type = EXT_FLASH_CMD_BLOCK_ERASE_64K;

    // Build the command
    uint8_t cmd[4] = { type, ( address >> 16 ) & 0xFF, ( address >> 8 ) & 0xFF, address & 0xFF };

    // Do the transfer
    p_intf->select();
    int rtn = p_intf->write( cmd, sizeof( cmd ) );
    p_intf->deselect();

    // Block while the erase is committed
    while( emb_ext_flash_busy( p_intf ) )
        ;

    // Return 0 if successful -1 otherwise
    return rtn;
}

int emb_ext_flash_chip_erase( emb_flash_intf_handle_t *p_intf )
{
    // Enable writes
    emb_ext_flash_write_enable( p_intf );

    // Null check
    if( !p_intf )
        return;

    // Build the command
    uint8_t cmd = EXT_FLASH_CMD_CHIP_ERASE;

    // Do the transfer
    p_intf->select();
    int rtn = p_intf->write( &cmd, 1 );
    p_intf->deselect();

    // Block while the erase is committed - this can take 2 minutes + on a chip a erase
    while( emb_ext_flash_busy( p_intf ) )
        ;

    // Return 0 if successful -1 otherwise
    return rtn;
}

uint8_t emb_ext_flash_get_status( emb_flash_intf_handle_t *p_intf )
{
    // Build the command
    uint8_t cmd = EXT_FLASH_CMD_READ_STATUS_REG;
    uint8_t status = 0;

    // Null check
    if( !p_intf )
        return status;

    // Do the transfer
    p_intf->select();
    p_intf->write( &cmd, 1 );
    p_intf->read( &status, 1 );
    p_intf->deselect();

    // Return the status
    return status;
}

int emb_ext_flash_sleep( emb_flash_intf_handle_t *p_intf )
{
    // Build the  command
    uint8_t cmd = EXT_FLASH_CMD_POWER_DOWN;

    // Null check
    if( !p_intf )
        return -1;

    // Do the transfer
    p_intf->select();
    int rtn = p_intf->write( &cmd, 1 );
    p_intf->deselect();

    return rtn;
}

int emb_ext_flash_wake( emb_flash_intf_handle_t *p_intf )
{
    // Build the command
    uint8_t cmd = EXT_FLASH_CMD_RELEASE_POWER_DOWN;

    // Null check
    if( !p_intf )
        return -1;

    // Do the transfer
    p_intf->select();
    int rtn = p_intf->write( &cmd, 1 );
    p_intf->deselect();

    // Wait for the specified duration for wake time - this can be optimized to your use case.
    p_intf->delay_us( 3 );

    // Return 0 for success -1 otherwise.
    return rtn;
}

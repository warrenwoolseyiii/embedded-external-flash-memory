#include "emb_ext_flash.h"

// Private functions
uint8_t emb_ext_flash_busy( int id )
{
    return ( emb_ext_flash_get_status( id ) & EXT_FLASH_STATUS_REG_BUSY );
}

void emb_ext_flash_write_enable( int id )
{
    uint8_t cmd = EXT_FLASH_CMD_WRITE_ENABLE;
    emb_ext_flash_transfer( id, &cmd, NULL, 1 );
    while( !( emb_ext_flash_get_status( id ) & EXT_FLASH_STATUS_REG_WEL ) )
        ;
}

// Pubic functions
int emb_ext_flash_get_jedec_id( int id, uint8_t *manufacturer_id, uint8_t *memory_type, uint8_t *capacity )
{
    uint8_t cmd = EXT_FLASH_CMD_JEDEC_ID;
    uint8_t data[3] = { 0 };
    emb_ext_flash_transfer( id, &cmd, data, 4 );
    *manufacturer_id = data[1];
    *memory_type = data[2];
    *capacity = data[3];
    return 0;
}

int emb_ext_flash_read( int id, uint32_t address, uint8_t *data, uint16_t len )
{
    uint8_t cmd[4] = { EXT_FLASH_CMD_READ_DATA, ( address >> 16 ) & 0xFF, ( address >> 8 ) & 0xFF, address & 0xFF };
    int     rtn = emb_ext_flash_transfer( id, cmd, data, len + 4 );
    return rtn - 4;
}

int emb_ext_flash_write( int id, uint32_t address, uint8_t *data, uint16_t len )
{
    emb_ext_flash_write_enable( id );
    uint8_t cmd[4] = { EXT_FLASH_CMD_PAGE_PROGRAM, ( address >> 16 ) & 0xFF, ( address >> 8 ) & 0xFF, address & 0xFF };
    int     rtn = emb_ext_flash_transfer( id, cmd, data, len + 4 );
    while( emb_ext_flash_busy( id ) )
        ;
    return rtn - 4;
}

int emb_ext_flash_erase( int id, uint32_t address, uint32_t len )
{
    emb_ext_flash_write_enable( id );
    uint8_t type = EXT_FLASH_CMD_SECTOR_ERASE;
    if( len > 4096 )
        type = EXT_FLASH_CMD_BLOCK_ERASE_32K;
    if( len > 32768 )
        type = EXT_FLASH_CMD_BLOCK_ERASE_64K;
    uint8_t cmd[4] = { type, ( address >> 16 ) & 0xFF, ( address >> 8 ) & 0xFF, address & 0xFF };
    int     rtn = emb_ext_flash_transfer( id, cmd, NULL, 4 );
    while( emb_ext_flash_busy( id ) )
        ;
    return rtn - 4;
}

void emb_ext_flash_chip_erase( int id )
{
    emb_ext_flash_write_enable( id );
    uint8_t cmd = EXT_FLASH_CMD_CHIP_ERASE;
    emb_ext_flash_transfer( id, &cmd, NULL, 1 );
    while( emb_ext_flash_busy( id ) )
        ;
}

uint8_t emb_ext_flash_get_status( int id )
{
    uint8_t cmd = EXT_FLASH_CMD_READ_STATUS_REG;
    uint8_t status = 0;
    emb_ext_flash_transfer( id, &cmd, &status, 2 );
    return status;
}

int emb_ext_flash_sleep( int id )
{
    uint8_t cmd = EXT_FLASH_CMD_POWER_DOWN;
    emb_ext_flash_transfer( id, &cmd, NULL, 1 );
    return 0;
}

int emb_ext_flash_wake( int id )
{}
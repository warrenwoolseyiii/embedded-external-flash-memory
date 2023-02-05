
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

#include <gtest/gtest.h>
#include <emb_ext_flash.h>

// Flash simulation JEDEC ID
#define FLASH_SIM_JEDEC_ID 0x1F4401

// Flash simulation memory size
#define FLASH_SIM_MEM_SIZE 0x40000

// Flash sim memory bank, default to 0xFF
uint8_t _flash_sim_mem[FLASH_SIM_MEM_SIZE] = { 0xFF };

// Flash simulation state enumaration
enum flash_sim_state
{
    FLASH_SIM_STATE_IDLE,
    FLASH_SIM_SET_ADDR,
    FLASH_SIM_STATE_READ,
    FLASH_SIM_STATE_WRITE,
    FLASH_SIM_STATUS_REG_READ,
    FLASH_SIM_STATUS_REG_WRITE,
    FLASH_SIM_ERASE,
    FLASH_SIM_GET_JEDEC_ID,
};

// Flash simulation state
int _flash_sim_state = FLASH_SIM_STATE_IDLE;

// Flash simulation current address
uint32_t _flash_sim_addr = 0;

// Flash simulation write enable latch
bool _flash_sim_wel = false;

// Flash simulation status register
uint8_t _flash_sim_status_reg = 0;

// Flash simulation erase length setting
uint32_t _flash_sim_erase_len = 0;

// Parse the command, return 0 if successful, -1 if not.
int flash_sim_parse_cmd( uint8_t cmd )
{
    // Switch based on the command
    switch( cmd ) {
        case EXT_FLASH_CMD_WRITE_ENABLE:
            // Set the write enable latch
            _flash_sim_wel = true;
            // Set the state to address setting
            _flash_sim_state = FLASH_SIM_SET_ADDR;
            // Set the status register to EXT_FLASH_STATUS_REG_WEL
            _flash_sim_status_reg |= EXT_FLASH_STATUS_REG_WEL;
            break;
        case EXT_FLASH_CMD_WRITE_DISABLE:
            // Clear the write enable latch
            _flash_sim_wel = false;
            break;
        case EXT_FLASH_CMD_READ_STATUS_REG:
            // Set the state to read the status register
            _flash_sim_state = FLASH_SIM_STATUS_REG_READ;
            break;
        case EXT_FLASH_CMD_WRITE_STATUS_REG:
            // Set the state to write the status register
            _flash_sim_state = FLASH_SIM_STATUS_REG_WRITE;
            break;
        case EXT_FLASH_CMD_READ_DATA:
            // Set the state to address setting
            _flash_sim_state = FLASH_SIM_SET_ADDR;
            break;
        case EXT_FLASH_CMD_FAST_READ:
            // Set the state to address setting
            _flash_sim_state = FLASH_SIM_SET_ADDR;
            break;
        case EXT_FLASH_CMD_PAGE_PROGRAM:
            // If write enable latch is set, set the state to address setting, otherwise just ignore the command
            if( _flash_sim_wel ) {
                _flash_sim_state = FLASH_SIM_SET_ADDR;
            }
            else {
                _flash_sim_state = FLASH_SIM_STATE_IDLE;
            }
            break;
        case EXT_FLASH_CMD_SECTOR_ERASE:
            // If write enable latch is set, set the state to address setting, otherwise just ignore the command
            if( _flash_sim_wel ) {
                _flash_sim_state = FLASH_SIM_SET_ADDR;
                _flash_sim_erase_len = 4096;
            }
            else {
                _flash_sim_state = FLASH_SIM_STATE_IDLE;
                _flash_sim_erase_len = 0;
            }
            break;
        case EXT_FLASH_CMD_BLOCK_ERASE_32K:
            // If write enable latch is set, set the state to address setting, otherwise just ignore the command
            if( _flash_sim_wel ) {
                _flash_sim_state = FLASH_SIM_SET_ADDR;
                _flash_sim_erase_len = 32768;
            }
            else {
                _flash_sim_state = FLASH_SIM_STATE_IDLE;
                _flash_sim_erase_len = 0;
            }
            break;
        case EXT_FLASH_CMD_BLOCK_ERASE_64K:
            // If write enable latch is set, set the state to address setting, otherwise just ignore the command
            if( _flash_sim_wel ) {
                _flash_sim_state = FLASH_SIM_SET_ADDR;
                _flash_sim_erase_len = 65536;
            }
            else {
                _flash_sim_state = FLASH_SIM_STATE_IDLE;
                _flash_sim_erase_len = 0;
            }
            break;
        case EXT_FLASH_CMD_CHIP_ERASE:
            // If write enable latch is set, set the state to address setting, otherwise just ignore the command
            if( _flash_sim_wel ) {
                _flash_sim_erase_len = FLASH_SIM_MEM_SIZE;
                memset( _flash_sim_mem, 0xFF, FLASH_SIM_MEM_SIZE );
            }
            else {
                _flash_sim_state = FLASH_SIM_STATE_IDLE;
                _flash_sim_erase_len = 0;
            }
            break;
        case EXT_FLASH_CMD_POWER_DOWN:
            // Do nothing
            break;
        case EXT_FLASH_CMD_RELEASE_POWER_DOWN:
            // Do nothing
            break;
        case EXT_FLASH_CMD_JEDEC_ID:
            // Set the state to get the JEDEC ID
            _flash_sim_state = FLASH_SIM_GET_JEDEC_ID;
            break;
        default:
            // Unknown command, set to IDLE
            _flash_sim_state = FLASH_SIM_STATE_IDLE;
            // Return an error
            return -1;
    }
    // Return success
    return 0;
}

// Flash simulation address setter
int flash_sim_set_addr( uint8_t next_byte )
{
    // Each address is 3 bytes long, passed in a byte at a time. The first byte is the most significant byte.
    // The address is stored in the flash_sim_addr variable.
    // Shift the address left by 8 bits and add the next byte, if its the first byte, clear the address first.
    static uint8_t addr_byte = 0;
    if( addr_byte == 0 ) {
        _flash_sim_addr = 0;
    }
    _flash_sim_addr = ( _flash_sim_addr << 8 ) | next_byte;
    addr_byte++;

    // If the address is 3 bytes long, return success
    if( addr_byte == 3 ) {
        addr_byte = 0;
        // Make sure the address is within the flash memory range by modulating it
        _flash_sim_addr %= FLASH_SIM_MEM_SIZE;
        return 0;
    }

    // Otherwise return failure
    return -1;
}

// Flash simulation jedec id getter
uint8_t flash_sim_get_jedec_id( void )
{
    // The jedec id is a 3 byte value. Return the next byte in the sequence, and reset the state when done.
    static uint8_t jedec_id_byte = 0;
    uint8_t        ret = FLASH_SIM_JEDEC_ID >> ( 8 * ( 2 - jedec_id_byte++ ) );
    if( jedec_id_byte == 3 ) {
        jedec_id_byte = 0;
    }
    return ret;
}

// Flash simulation state machine
uint8_t flash_sim_sm( uint8_t next_byte )
{
    // Switch based on the state
    switch( _flash_sim_state ) {
        case FLASH_SIM_STATE_IDLE:
            // Parse the command, regardless of the result return 0xFF
            flash_sim_parse_cmd( next_byte );
            return 0xFF;
            break;
        case FLASH_SIM_SET_ADDR:
            // Parse the address, if its successful set the state to read or write depending on the WEL, return 0xFF regardless
            if( flash_sim_set_addr( next_byte ) == 0 ) {
                if( _flash_sim_wel && _flash_sim_erase_len == 0 ) {
                    _flash_sim_state = FLASH_SIM_STATE_WRITE;
                }
                else if( _flash_sim_wel && _flash_sim_erase_len > 0 ) {
                    // Erase the address space specified by the current address and the erase length
                    for( uint32_t i = 0; i < _flash_sim_erase_len; i++ ) {
                        _flash_sim_mem[( _flash_sim_addr + i ) % FLASH_SIM_MEM_SIZE] = 0xFF;
                    }
                    // Set the status register to busy
                    _flash_sim_status_reg |= EXT_FLASH_STATUS_REG_BUSY;
                    // Clear the WEL in the status register
                    _flash_sim_status_reg &= ~EXT_FLASH_STATUS_REG_WEL;
                    _flash_sim_wel = false;
                }
                else {
                    _flash_sim_state = FLASH_SIM_STATE_READ;
                }
            }
            return 0xFF;
            break;
        case FLASH_SIM_STATE_READ: {
            // Return the next byte of the read
            uint8_t ret = _flash_sim_mem[_flash_sim_addr];
            // Increment the address, protect against overflow
            _flash_sim_addr = ( _flash_sim_addr + 1 ) % FLASH_SIM_MEM_SIZE;
            return ret;
        } break;
        case FLASH_SIM_STATE_WRITE:
            // Write the next byte to the memory by AND'ing it with the byte
            _flash_sim_mem[_flash_sim_addr] &= next_byte;
            // Increment the address, protect against overflow
            _flash_sim_addr = ( _flash_sim_addr + 1 ) % FLASH_SIM_MEM_SIZE;
            // Make sure the status byte is set to write in progress
            _flash_sim_status_reg |= EXT_FLASH_STATUS_REG_BUSY;
            // Clear the WEL in the status register
            _flash_sim_status_reg &= ~EXT_FLASH_STATUS_REG_WEL;
            _flash_sim_wel = false;
            return 0xFF;
            break;
        case FLASH_SIM_STATUS_REG_READ:
            // Return the status register
            return _flash_sim_status_reg;
            break;
        case FLASH_SIM_STATUS_REG_WRITE:
            // Write the next byte to the status register
            _flash_sim_status_reg = next_byte;
            break;
        case FLASH_SIM_GET_JEDEC_ID:
            // Return the next byte of the JEDEC ID
            return flash_sim_get_jedec_id();
            break;
        default:
            // Unknown state, set to IDLE
            _flash_sim_state = FLASH_SIM_STATE_IDLE;
            break;
    }

    // Return the next byte to send
    return 0xFF;
}

// Interface "selected" flag
bool _is_selected = false;

// Interface select method
void _select()
{
    _is_selected = true;
}

// Interface deselect method
void _deselect()
{
    _is_selected = false;
    // Set the state to idle
    _flash_sim_state = FLASH_SIM_STATE_IDLE;
    // Set the erase length to 0
    _flash_sim_erase_len = 0;
    // Clear the busy bit in the status register
    _flash_sim_status_reg &= ~EXT_FLASH_STATUS_REG_BUSY;
    // Set the address to 0
    _flash_sim_addr = 0;
}

// Interface buffer write method, returns 0 if successful, -1 if not.
int _write( uint8_t *data, uint16_t len )
{
    // Run the flash simulation state machine for each byte in the buffer
    for( uint16_t i = 0; i < len; i++ ) {
        flash_sim_sm( data[i] );
    }
    return 0;
}

// Interface buffer read method, returns 0 if successful, -1 if not.
int _read( uint8_t *data, uint16_t len )
{
    // Run the flash simulation state machine for each byte in the buffer
    for( uint16_t i = 0; i < len; i++ ) {
        data[i] = flash_sim_sm( 0xFF );
    }
    return 0;
}

// Interface delay method for a specified duration in microseconds.
void _delay_us( uint32_t duration )
{
    // Do nothing
}

// Make a global interface struct
emb_flash_intf_handle_t _intf = {
    false,
    _select,
    _deselect,
    _write,
    _read,
    _delay_us };

// Class for facilitating embedded external flash memory tests
class emb_ext_flash_test : public ::testing::Test
{
  public:
    emb_ext_flash_test()
    {
        // initialization code here
    }

    void SetUp()
    {
        // code here will execute just before the test ensues
        // Force the interface struct to pass through
        emb_ext_flash_init_intf( &_intf );
        // Deselect the interface
        _intf.deselect();
        _flash_sim_wel = false;
    }

    void TearDown()
    {
        // code here will be called just after the test completes
        // ok to through exceptions from here if need be
    }

    ~emb_ext_flash_test()
    {
        // cleanup any pending stuff, but no exceptions allowed
    }

    // put in any custom data members that you need
};

TEST_F( emb_ext_flash_test, print_version )
{
    printf( "emb_ext_flash version: %s\n", emb_ext_flash_get_lib_ver() );
    ASSERT_TRUE( 1 );
}

TEST_F( emb_ext_flash_test, test_null_init )
{
    // Test null initialization
    ASSERT_EQ( emb_ext_flash_init_intf( NULL ), -1 );

    // Test null interface
    emb_flash_intf_handle_t intf = { 0 };
    ASSERT_EQ( emb_ext_flash_init_intf( &intf ), -1 );

    // Build a complete interface struct
    emb_flash_intf_handle_t intf2 = {
        false,
        _select,
        _deselect,
        _write,
        _read,
        _delay_us };

    // Test null select method
    intf2.select = NULL;
    ASSERT_EQ( emb_ext_flash_init_intf( &intf2 ), -1 );
    ASSERT_FALSE( intf2.initialized );
    intf2.select = _select;

    // Test null deselect method
    intf2.deselect = NULL;
    ASSERT_EQ( emb_ext_flash_init_intf( &intf2 ), -1 );
    ASSERT_FALSE( intf2.initialized );
    intf2.deselect = _deselect;

    // Test null write method
    intf2.write = NULL;
    ASSERT_EQ( emb_ext_flash_init_intf( &intf2 ), -1 );
    ASSERT_FALSE( intf2.initialized );
    intf2.write = _write;

    // Test null read method
    intf2.read = NULL;
    ASSERT_EQ( emb_ext_flash_init_intf( &intf2 ), -1 );
    ASSERT_FALSE( intf2.initialized );
    intf2.read = _read;

    // Test null delay method
    intf2.delay_us = NULL;
    ASSERT_EQ( emb_ext_flash_init_intf( &intf2 ), -1 );
    ASSERT_FALSE( intf2.initialized );
    intf2.delay_us = _delay_us;
}

TEST_F( emb_ext_flash_test, test_init )
{
    // Test initialization
    _intf.initialized = false;
    ASSERT_EQ( emb_ext_flash_init_intf( &_intf ), 0 );
    ASSERT_TRUE( _intf.initialized );
}

TEST_F( emb_ext_flash_test, test_null_intf_checks )
{
    // Test jedec id check
    ASSERT_EQ( emb_ext_flash_get_jedec_id( NULL, NULL, NULL, NULL ), -1 );
    ASSERT_EQ( emb_ext_flash_get_jedec_id( &_intf, NULL, NULL, NULL ), -1 );
    uint8_t manufacturer_id = 0;
    ASSERT_EQ( emb_ext_flash_get_jedec_id( &_intf, &manufacturer_id, NULL, NULL ), -1 );
    uint8_t memory_type = 0;
    ASSERT_EQ( emb_ext_flash_get_jedec_id( &_intf, &manufacturer_id, &memory_type, NULL ), -1 );

    // Test read checks
    ASSERT_EQ( emb_ext_flash_read( NULL, 0, NULL, 0 ), 0 );
    ASSERT_EQ( emb_ext_flash_read( &_intf, 0, NULL, 0 ), 0 );

    // Test write checks
    ASSERT_EQ( emb_ext_flash_write( NULL, 0, NULL, 0 ), 0 );
    ASSERT_EQ( emb_ext_flash_write( &_intf, 0, NULL, 0 ), 0 );

    // Test erase checks
    ASSERT_EQ( emb_ext_flash_erase( NULL, 0, 0 ), -1 );

    // Test chip erase checks
    ASSERT_EQ( emb_ext_flash_chip_erase( NULL ), -1 );

    // Test status register checks
    ASSERT_EQ( emb_ext_flash_get_status( NULL ), 0 );

    // Test write sleep checks
    ASSERT_EQ( emb_ext_flash_sleep( NULL ), -1 );

    // Test wake checks
    ASSERT_EQ( emb_ext_flash_wake( NULL ), -1 );
}

TEST_F( emb_ext_flash_test, test_jedec_id )
{
    // Test jedec id against the simulator
    uint8_t manufacturer_id = 0;
    uint8_t memory_type = 0;
    uint8_t capacity = 0;
    ASSERT_EQ( emb_ext_flash_get_jedec_id( &_intf, &manufacturer_id, &memory_type, &capacity ), 0 );
    ASSERT_EQ( manufacturer_id, FLASH_SIM_JEDEC_ID >> 16 );
    ASSERT_EQ( memory_type, ( FLASH_SIM_JEDEC_ID >> 8 ) & 0xFF );
    ASSERT_EQ( capacity, FLASH_SIM_JEDEC_ID & 0xFF );
}

TEST_F( emb_ext_flash_test, test_read_exclude_payload )
{
    // Test a blind read against the simulator
    uint8_t data[256];
    ASSERT_EQ( emb_ext_flash_read( &_intf, 0, data, 256 ), 256 );

    // Read around the edges
    ASSERT_EQ( emb_ext_flash_read( &_intf, FLASH_SIM_MEM_SIZE - 1, data, 256 ), 256 );
}

TEST_F( emb_ext_flash_test, test_erase_write_read )
{
    // Address to write to
    uint32_t addr = 0;
    // Tx data
    uint8_t tx_data[256];
    // Rx data
    uint8_t rx_data[256] = { 0 };

    // Initialize the tx data
    for( int i = 0; i < 256; i++ ) {
        tx_data[i] = i;
    }

    // Erase the sector
    ASSERT_EQ( emb_ext_flash_erase( &_intf, addr, 256 ), 0 );

    // Write the data
    ASSERT_EQ( emb_ext_flash_write( &_intf, addr, tx_data, 256 ), 256 );

    // Read the data
    ASSERT_EQ( emb_ext_flash_read( &_intf, addr, rx_data, 256 ), 256 );

    // Compare the data
    ASSERT_EQ( memcmp( tx_data, rx_data, 256 ), 0 );
}

TEST_F( emb_ext_flash_test, test_boundary_write_read )
{
    // Do a chip erase first
    ASSERT_EQ( emb_ext_flash_chip_erase( &_intf ), 0 );

    // Address to write to
    uint32_t addr = FLASH_SIM_MEM_SIZE - 128;

    // Tx data
    uint8_t tx_data[256];
    // Rx data
    uint8_t rx_data[256] = { 0 };

    // Initialize the tx data
    for( int i = 0; i < 256; i++ ) {
        tx_data[i] = i;
    }

    // Write the data
    ASSERT_EQ( emb_ext_flash_write( &_intf, addr, tx_data, 256 ), 256 );

    // Read the data
    ASSERT_EQ( emb_ext_flash_read( &_intf, addr, rx_data, 256 ), 256 );

    // Compare the data
    ASSERT_EQ( memcmp( tx_data, rx_data, 256 ), 0 );
}

TEST_F( emb_ext_flash_test, test_sim_overwrite )
{
    // Sector erase at address 0
    ASSERT_EQ( emb_ext_flash_erase( &_intf, 0, 256 ), 0 );

    // Write 256 bytes at address 0
    uint8_t tx_data[256];
    for( int i = 0; i < 256; i++ ) {
        tx_data[i] = i;
    }

    ASSERT_EQ( emb_ext_flash_write( &_intf, 0, tx_data, 256 ), 256 );

    // Read 256 bytes at address 0
    uint8_t rx_data[256] = { 0 };
    ASSERT_EQ( emb_ext_flash_read( &_intf, 0, rx_data, 256 ), 256 );

    // Compare the data
    ASSERT_EQ( memcmp( tx_data, rx_data, 256 ), 0 );

    // Write 256 bytes at address 0
    for( int i = 0; i < 256; i++ ) {
        tx_data[i] = i + 1;
    }

    ASSERT_EQ( emb_ext_flash_write( &_intf, 0, tx_data, 256 ), 256 );

    // Read 256 bytes at address 0
    memset( rx_data, 0, 256 );
    ASSERT_EQ( emb_ext_flash_read( &_intf, 0, rx_data, 256 ), 256 );

    // Compare the data, it should not be equal
    ASSERT_NE( memcmp( tx_data, rx_data, 256 ), 0 );
}

TEST_F( emb_ext_flash_test, sector_erase )
{
    // Write the entire memory to 0
    uint8_t tx_data[4096] = { 0 };
    ASSERT_EQ( emb_ext_flash_write( &_intf, 0, tx_data, 4096 ), 4096 );

    // Read back the sector
    ASSERT_EQ( emb_ext_flash_read( &_intf, 0, tx_data, 4096 ), 4096 );

    // Ensure the sector is 0
    for( int i = 0; i < 4096; i++ ) {
        ASSERT_EQ( tx_data[i], 0 );
    }

    // Erase the sector
    ASSERT_EQ( emb_ext_flash_erase( &_intf, 0, 4096 ), 0 );

    // Read back the sector
    ASSERT_EQ( emb_ext_flash_read( &_intf, 0, tx_data, 4096 ), 4096 );

    // Ensure the sector is 0xFF
    for( int i = 0; i < 4096; i++ ) {
        ASSERT_EQ( tx_data[i], 0xFF );
    }
}

TEST_F( emb_ext_flash_test, block_32k_erase )
{
    // Write the entire memory to 0
    uint8_t tx_data[32768] = { 0 };
    ASSERT_EQ( emb_ext_flash_write( &_intf, 0, tx_data, 32768 ), 32768 );

    // Read back the sector
    ASSERT_EQ( emb_ext_flash_read( &_intf, 0, tx_data, 32768 ), 32768 );

    // Ensure the sector is 0
    for( int i = 0; i < 32768; i++ ) {
        ASSERT_EQ( tx_data[i], 0 );
    }

    // Erase the sector
    ASSERT_EQ( emb_ext_flash_erase( &_intf, 0, 32768 ), 0 );

    // Read back the sector
    ASSERT_EQ( emb_ext_flash_read( &_intf, 0, tx_data, 32768 ), 32768 );

    // Ensure the sector is 0xFF
    for( int i = 0; i < 32768; i++ ) {
        ASSERT_EQ( tx_data[i], 0xFF );
    }
}

TEST_F( emb_ext_flash_test, block_64k_erase )
{
    // Write the entire memory to 0
    uint8_t  tx_data[32768] = { 0 };
    uint32_t addr = 0;
    for( int i = 0; i < 2; i++ ) {
        ASSERT_EQ( emb_ext_flash_write( &_intf, addr, tx_data, 32768 ), 32768 );
        addr += 32768;
    }

    // Read back the sector
    addr = 0;
    for( int i = 0; i < 2; i++ ) {
        ASSERT_EQ( emb_ext_flash_read( &_intf, addr, tx_data, 32768 ), 32768 );
        // Ensure the sector is 0
        for( int j = 0; j < 32768; j++ ) {
            ASSERT_EQ( tx_data[j], 0 );
        }
        addr += 32768;
    }

    // Erase the sector
    ASSERT_EQ( emb_ext_flash_erase( &_intf, 0, 65536 ), 0 );

    // Read back the sector
    addr = 0;
    for( int i = 0; i < 2; i++ ) {
        ASSERT_EQ( emb_ext_flash_read( &_intf, addr, tx_data, 32768 ), 32768 );
        // Ensure the sector is 0xFF
        for( int j = 0; j < 32768; j++ ) {
            ASSERT_EQ( tx_data[j], 0xFF );
        }
        addr += 32768;
    }
}
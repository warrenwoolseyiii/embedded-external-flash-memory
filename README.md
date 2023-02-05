# External Flash Memory Library
A library for interfacing with an external flash memory chip. The library provides a set of functions for reading, writing, and erasing data on an external flash memory chip.

# Usage
The user must implement their own functions for communication with the external flash memory chip and assign them to an instance of the `emb_flash_intf_handle_t` handle struct.

```
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
```

The user must then call the `emb_ext_flash_init_intf` function to initialize the handle struct properly. Multiple handle structs can be utilized.

## Features
The library offers the following functions to the user:

- `int emb_ext_flash_get_jedec_id( emb_flash_intf_handle_t *p_intf, uint8_t *manufacturer_id, uint8_t *memory_type, uint8_t *capacity )`: gets the JEDEC ID of the external flash memory chip.

- `int emb_ext_flash_read( emb_flash_intf_handle_t *p_intf, uint32_t address, uint8_t *data, uint16_t len )`: reads data from the external flash memory chip.

- `int emb_ext_flash_write( emb_flash_intf_handle_t *p_intf, uint32_t address, uint8_t *data, uint16_t len )`: writes data to the external flash memory chip.

- `int emb_ext_flash_erase( emb_flash_intf_handle_t *p_intf, uint32_t address, uint32_t len )`: erases data from the external flash memory chip.

- `int emb_ext_flash_chip_erase( emb_flash_intf_handle_t *p_intf )`: erases the entire external flash memory chip.

- `uint8_t emb_ext_flash_get_status( emb_flash_intf_handle_t *p_intf )`: reads the status register of the external flash memory chip.

- `int emb_ext_flash_sleep( emb_flash_intf_handle_t *p_intf )`: puts the external flash memory chip into sleep mode.

- `int emb_ext_flash_wake( emb_flash_intf_handle_t *p_intf )`: wakes the external flash memory chip from sleep mode.

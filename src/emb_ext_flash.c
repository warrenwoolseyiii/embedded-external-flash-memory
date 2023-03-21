/*********************************************************************************
*  DISCLAIMER:
*
*  This code is protected under the MIT open source license. The code is provided
*  "as is" without warranty of any kind, either express or implied, including but
*  not limited to the implied warranties of merchantability, fitness for a particular
*  purpose, or non-infringement. In no event shall the author or any other party be
*  liable for any direct, indirect, incidental, special, exemplary, or consequential
*  damages, however caused and on any theory of liability, whether in contract,
*  strict liability, or tort (including negligence or otherwise), arising in any way
*  out of the use of this code or performance or use of the results of this code. By
*  using this code, you agree to hold the author and any other party harmless from
*  any and all liability and to use the code at your own risk.
*
*  This code was written by GitHub user: budgettsfrog
*  Contact: budgettsfrog@protonmail.com
*  GitHub: https://github.com/warrenwoolseyiii
*********************************************************************************/

#include "emb_ext_flash.h"
#include "emb_ext_flash_version.h"

// Private functions
uint8_t emb_ext_flash_busy(emb_flash_intf_handle_t *p_intf)
{
   return(emb_ext_flash_get_status(p_intf) & EXT_FLASH_STATUS_REG_BUSY);
}

void emb_ext_flash_write_enable(emb_flash_intf_handle_t *p_intf)
{
   // Make the command
   uint8_t cmd = EXT_FLASH_CMD_WRITE_ENABLE;

   // Null check
   if (!p_intf || !p_intf->initialized)
   {
      return;
   }

   // Do the transfer
   p_intf->select();
   p_intf->write(&cmd, 1);
   p_intf->deselect();

   // Block while the WEL bit in the status register is unset
   while (!(emb_ext_flash_get_status(p_intf) & EXT_FLASH_STATUS_REG_WEL))
   {
      ;
   }
}

// Pubic functions
int emb_ext_flash_init_intf(emb_flash_intf_handle_t *p_intf)
{
   // Null check
   if (!p_intf)
   {
      return(-1);
   }

   // Set the initialized flag to 0;
   p_intf->initialized = 0;

   // Check all of the function pointers, if any are null return -1
   if (!p_intf->select || !p_intf->deselect || !p_intf->write || !p_intf->read || !p_intf->delay_us)
   {
      return(-1);
   }

   // Set the initialized flag to 1
   p_intf->initialized = 1;

   return(0);
}

int emb_ext_flash_get_jedec_id(emb_flash_intf_handle_t *p_intf, uint8_t *manufacturer_id, uint8_t *memory_type, uint8_t *capacity)
{
   // Build the command and the payload
   uint8_t cmd     = EXT_FLASH_CMD_JEDEC_ID;
   uint8_t data[3] = { 0 };

   // Null check
   if (!p_intf || !p_intf->initialized || !manufacturer_id || !memory_type || !capacity)
   {
      return(-1);
   }

   // Do the transfer
   p_intf->select();
   p_intf->write(&cmd, 1);
   int rtn = p_intf->read(data, 3);
   p_intf->deselect();

   // Populate the fields
   *manufacturer_id = data[0];
   *memory_type     = data[1];
   *capacity        = data[2];

   // Return 0 for non error, -1 for error
   return(rtn);
}

int emb_ext_flash_read(emb_flash_intf_handle_t *p_intf, uint32_t address, uint8_t *data, uint16_t len)
{
   // Build the command
   uint8_t cmd[4] = { EXT_FLASH_CMD_READ_DATA, (address >> 16) & 0xFF, (address >> 8) & 0xFF, address & 0xFF };

   // Null check
   if (!p_intf || !p_intf->initialized || !data || !len)
   {
      return(0);
   }

   // Do the transfer
   p_intf->select();
   p_intf->write(cmd, sizeof(cmd));
   int rtn = p_intf->read(data, len);
   p_intf->deselect();

   // Return the number of bytes read
   return(rtn == 0 ? len : 0);
}

int emb_ext_flash_write(emb_flash_intf_handle_t *p_intf, uint32_t address, uint8_t *data, uint16_t len)
{
   uint16_t t_len         = len;
   int      rtn           = 0;
   int      bytes_written = 0;

   do
   {
      // Enable writes
      emb_ext_flash_write_enable(p_intf);

      // Build the command
      uint8_t cmd[4] = { EXT_FLASH_CMD_PAGE_PROGRAM, (address >> 16) & 0xFF, (address >> 8) & 0xFF, address & 0xFF };

      // Null check
      if (!p_intf || !p_intf->initialized || !data || !len)
      {
         return(0);
      }

      // If the address + the length is going to cross a 256 byte page boundary, we need to split this into 2 transactions.
      int w_len = len;
      if ((address & 0xFF) + len > 0xFF)
      {
         w_len = 0x100 - (address & 0xFF);
      }

      // Do the transfer
      p_intf->select();
      p_intf->write(cmd, sizeof(cmd));
      rtn = p_intf->write(data, w_len);
      p_intf->deselect();

      // Block while the flash chip commits the write
      while (emb_ext_flash_busy(p_intf))
      {
         ;
      }

      // Keep track of the number of bytes written
      bytes_written += w_len;
      if (rtn != 0)
      {
         break;
      }

      if (bytes_written < t_len)
      {
         // Update the address and data pointer if we need to
         address += w_len;
         data    += w_len;
         len     -= w_len;
      }
   } while (bytes_written < t_len);

   // Return the number of bytes written
   return(bytes_written);
}

int emb_ext_flash_erase(emb_flash_intf_handle_t *p_intf, uint32_t address, uint32_t len)
{
   // Enable writes
   emb_ext_flash_write_enable(p_intf);

   // Null check
   if (!p_intf || !p_intf->initialized)
   {
      return(-1);
   }

   // Determine the most efficient command to use
   uint8_t type = EXT_FLASH_CMD_SECTOR_ERASE;
   if (len > 4096)
   {
      type = EXT_FLASH_CMD_BLOCK_ERASE_32K;
   }
   if (len > 32768)
   {
      type = EXT_FLASH_CMD_BLOCK_ERASE_64K;
   }

   // Build the command
   uint8_t cmd[4] = { type, (address >> 16) & 0xFF, (address >> 8) & 0xFF, address & 0xFF };

   // Do the transfer
   p_intf->select();
   int rtn = p_intf->write(cmd, sizeof(cmd));
   p_intf->deselect();

   // Block while the erase is committed
   while (emb_ext_flash_busy(p_intf))
   {
      ;
   }

   // Return 0 if successful -1 otherwise
   return(rtn);
}

int emb_ext_flash_chip_erase(emb_flash_intf_handle_t *p_intf)
{
   // Enable writes
   emb_ext_flash_write_enable(p_intf);

   // Null check
   if (!p_intf || !p_intf->initialized)
   {
      return(-1);
   }

   // Build the command
   uint8_t cmd = EXT_FLASH_CMD_CHIP_ERASE;

   // Do the transfer
   p_intf->select();
   int rtn = p_intf->write(&cmd, 1);
   p_intf->deselect();

   // Block while the erase is committed - this can take 2 minutes + on a chip a erase
   while (emb_ext_flash_busy(p_intf))
   {
      ;
   }

   // Return 0 if successful -1 otherwise
   return(rtn);
}

uint8_t emb_ext_flash_get_status(emb_flash_intf_handle_t *p_intf)
{
   // Build the command
   uint8_t cmd    = EXT_FLASH_CMD_READ_STATUS_REG;
   uint8_t status = 0;

   // Null check
   if (!p_intf || !p_intf->initialized)
   {
      return(status);
   }

   // Do the transfer
   p_intf->select();
   p_intf->write(&cmd, 1);
   p_intf->read(&status, 1);
   p_intf->deselect();

   // Return the status
   return(status);
}

int emb_ext_flash_sleep(emb_flash_intf_handle_t *p_intf)
{
   // Build the  command
   uint8_t cmd = EXT_FLASH_CMD_POWER_DOWN;

   // Null check
   if (!p_intf || !p_intf->initialized)
   {
      return(-1);
   }

   // Do the transfer
   p_intf->select();
   int rtn = p_intf->write(&cmd, 1);
   p_intf->deselect();

   return(rtn);
}

int emb_ext_flash_wake(emb_flash_intf_handle_t *p_intf)
{
   // Build the command
   uint8_t cmd = EXT_FLASH_CMD_RELEASE_POWER_DOWN;

   // Null check
   if (!p_intf || !p_intf->initialized)
   {
      return(-1);
   }

   // Do the transfer
   p_intf->select();
   int rtn = p_intf->write(&cmd, 1);
   p_intf->deselect();

   // Wait for the specified duration for wake time - this can be optimized to your use case.
   p_intf->delay_us(3);

   // Return 0 for success -1 otherwise.
   return(rtn);
}

const char *emb_ext_flash_get_lib_ver()
{
   // Get the major, minor, and rev numbers
   int major = EMBEDDED_FLASH_MAJOR;
   int minor = EMBEDDED_FLASH_MINOR;
   int rev   = EMBEDDED_FLASH_REV;

   // Build the string, without using sprintf
   static char ver[16];

   ver[0]  = 'v';
   ver[1]  = (major / 100) + '0';
   ver[2]  = ((major % 100) / 10) + '0';
   ver[3]  = (major % 10) + '0';
   ver[4]  = '.';
   ver[5]  = (minor / 100) + '0';
   ver[6]  = ((minor % 100) / 10) + '0';
   ver[7]  = (minor % 10) + '0';
   ver[8]  = '.';
   ver[9]  = (rev / 100) + '0';
   ver[10] = ((rev % 100) / 10) + '0';
   ver[11] = (rev % 10) + '0';
   ver[12] = '\0';

   // Return the string
   return(ver);
}

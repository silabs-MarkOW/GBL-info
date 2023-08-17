#include "app_assert.h"
#include "app_log.h"
#include "nvm3_default.h"
#include "btl_interface.h"
#include "btl_interface_storage.h"
#include "em_core.h"

#define mainBootloaderTable               (*(MainBootloaderTable_t **) \
                                           (BTL_MAIN_BOOTLOADER_TABLE_BASE))
#define Pp(X) app_log(#X ": %p\n",X)
#define Plx(X) app_log(#X ": %lx\n",X)
#define PCAP(X) app_log("  " #X ": %d\n",(mainBootloaderTable->capabilities & (X)) == X);

void show_bootloader(void)
{
  app_log_info("Bootloader and other flash related info\n");
  app_log_info("VTOR: %08lx\n",SCB->VTOR);
  app_log_info("NVM3: %p - %p\n", nvm3_defaultInit->nvmAdr, nvm3_defaultInit->nvmAdr + nvm3_defaultInit->nvmSize);
  Pp(mainBootloaderTable);
  Plx(mainBootloaderTable->header.type);
  Plx(mainBootloaderTable->header.layout);
  Plx(mainBootloaderTable->header.version);
  Plx(mainBootloaderTable->size);
  Pp(mainBootloaderTable->startOfAppSpace);
  Pp(mainBootloaderTable->endOfAppSpace);
  Plx(mainBootloaderTable->capabilities);
  PCAP(BOOTLOADER_CAPABILITY_ENFORCE_UPGRADE_SIGNATURE);
  PCAP(BOOTLOADER_CAPABILITY_ENFORCE_UPGRADE_ENCRYPTION);
  PCAP(BOOTLOADER_CAPABILITY_ENFORCE_SECURE_BOOT);
  PCAP(BOOTLOADER_CAPABILITY_BOOTLOADER_UPGRADE);
  PCAP(BOOTLOADER_CAPABILITY_GBL);
  PCAP(BOOTLOADER_CAPABILITY_GBL_SIGNATURE);
  PCAP(BOOTLOADER_CAPABILITY_GBL_ENCRYPTION);
  PCAP(BOOTLOADER_CAPABILITY_ENFORCE_CERTIFICATE_SECURE_BOOT);
  PCAP(BOOTLOADER_CAPABILITY_ROLLBACK_PROTECTION);
  PCAP(BOOTLOADER_CAPABILITY_PERIPHERAL_LIST);
  PCAP(BOOTLOADER_CAPABILITY_STORAGE);
  PCAP(BOOTLOADER_CAPABILITY_COMMUNICATION);
  Pp(mainBootloaderTable->init);
  Pp(mainBootloaderTable->deinit);
  Pp(mainBootloaderTable->verifyApplication);
  Pp(mainBootloaderTable->initParser);
  Pp(mainBootloaderTable->parseBuffer);
  Pp(mainBootloaderTable->storage);
  Pp(mainBootloaderTable->parseImageInfo);
  Pp(mainBootloaderTable->parserContextSize);
  Pp(mainBootloaderTable->remainingApplicationUpgrades);
  Pp(mainBootloaderTable->getPeripheralList);
  Pp(mainBootloaderTable->getUpgradeLocation);
  uint32_t rc;
  rc = bootloader_init();
  app_log("bootloader_init() returned 0x%lx\n",rc);
  BootloaderInformation_t info;
  BootloaderStorageInformation_t storageInfo;
  BootloaderStorageSlot_t slot;
  bootloader_getInfo(&info);
  app_log_info("Bootloader: capabilities:%08lx, type:%08x, version:%08lx\n",info.capabilities,info.type,info.version);
  if(info.capabilities & BOOTLOADER_CAPABILITY_STORAGE) {
      bootloader_getStorageInfo(&storageInfo);
      app_log_info("Bootloader storage: numStorageSlots:%ld\n",storageInfo.numStorageSlots);
      for(uint32_t i = 0; i < storageInfo.numStorageSlots; i++) {
          rc = bootloader_getStorageSlotInfo(i, &slot);
          app_assert(0 == rc,"bootloader_getStorageSlotInfo(%ld) returns 0x%lx",i, rc);
          app_log_info("Bootloader storage slot: 0x%08lx - 0x%08lx\n",slot.address, slot.address+slot.length);
      }
  }
}

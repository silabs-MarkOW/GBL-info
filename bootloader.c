#include "app_assert.h"
#include "app_log.h"
#include "nvm3_default.h"
#include "btl_interface.h"
#include "btl_interface_storage.h"
#include "btl_interface_parser.h"
#include "em_core.h"
#include <string.h>
#include <stdio.h>

#define mainBootloaderTable               (*(MainBootloaderTable_t **) \
                                           (BTL_MAIN_BOOTLOADER_TABLE_BASE))
#define Pp(X) app_log(#X ": %p\n",X)
#define Plx(X) app_log(#X ": 0x%lx\n",X)
#define PCAP(X) app_log("  " #X ": %d\n",(mainBootloaderTable->capabilities & (X)) == X);

bool isEmpty(uint32_t start, uint32_t length) {
  size_t word_count = length >> 2;
  uint32_t *words = (uint32_t *)start;
  for(size_t index = 0; index < word_count; index++) {
      if(0xffffffff != words[index]) {
          return false;
      }
  }
  return true;
}

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
#if defined(BOOTLOADER_CAPABILITY_GBL)
  PCAP(BOOTLOADER_CAPABILITY_GBL);
#endif
#if defined(BOOTLOADER_CAPABILITY_GBL_SIGNATURE)
  PCAP(BOOTLOADER_CAPABILITY_GBL_SIGNATURE);
#endif
#if defined(BOOTLOADER_CAPABILITY_GBL_ENCRYPTION)
  PCAP(BOOTLOADER_CAPABILITY_GBL_ENCRYPTION);
#endif
  PCAP(BOOTLOADER_CAPABILITY_ENFORCE_CERTIFICATE_SECURE_BOOT);
  PCAP(BOOTLOADER_CAPABILITY_ROLLBACK_PROTECTION);
#if defined(BOOTLOADER_CAPABILITY_PERIPHERAL_LIST)
  PCAP(BOOTLOADER_CAPABILITY_PERIPHERAL_LIST);
#endif
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
#if defined(BOOTLOADER_CAPABILITY_PERIPHERAL_LIST)
  Pp(mainBootloaderTable->getPeripheralList);
  Pp(mainBootloaderTable->getUpgradeLocation);
#endif
  uint32_t rc;
  rc = bootloader_init();
  app_log("bootloader_init() returned 0x%lx\n",rc);
  if(rc) return;
  BootloaderInformation_t info;
  BootloaderStorageInformation_t storageInfo;
  BootloaderStorageSlot_t slot;
  bootloader_getInfo(&info);
  app_log_info("Bootloader: capabilities:%08lx, type:%08x, version:%08lx\n",info.capabilities,info.type,info.version);
  if(info.capabilities & BOOTLOADER_CAPABILITY_STORAGE) {
      bootloader_getStorageInfo(&storageInfo);
      app_log_info("Bootloader storage: numStorageSlots:%ld\n",storageInfo.numStorageSlots);
      for(uint32_t i = 0; i < storageInfo.numStorageSlots; i++) {
          char status[32] = "unknown";
          rc = bootloader_getStorageSlotInfo(i, &slot);
          app_assert(0 == rc,"bootloader_getStorageSlotInfo(%ld) returns 0x%lx",i, rc);
          if(isEmpty(slot.address, slot.length)) {
              strcpy(status,"empty");
          } else {
              rc = bootloader_verifyImage(i, NULL);
              if(BOOTLOADER_OK == rc) {
                  strcpy(status, "valid image");
              } else {
                sprintf(status,"invalid: 0x%08lx",rc);
              }
          }
          app_log_info("Bootloader storage slot: 0x%08lx - 0x%08lx,: %s\n",
                       slot.address,
                       slot.address+slot.length,
                       status);
      }
  }
}

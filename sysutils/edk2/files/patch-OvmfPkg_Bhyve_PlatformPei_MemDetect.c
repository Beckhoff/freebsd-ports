--- OvmfPkg/Bhyve/PlatformPei/MemDetect.c.orig	2023-08-25 01:54:50 UTC
+++ OvmfPkg/Bhyve/PlatformPei/MemDetect.c
@@ -30,6 +30,7 @@ Module Name:
 #include <Library/PcdLib.h>
 #include <Library/PciLib.h>
 #include <Library/PeimEntryPoint.h>
+#include <Library/QemuFwCfgLib.h>
 #include <Library/ResourcePublicationLib.h>
 #include <Library/MtrrLib.h>
 
@@ -357,6 +358,9 @@ PublishPeiMemory (
   VOID
   )
 {
+  EFI_E820_ENTRY64      E820Entry;
+  FIRMWARE_CONFIG_ITEM  Item;
+  UINTN                 ItemSize;
   EFI_STATUS            Status;
   EFI_PHYSICAL_ADDRESS  MemoryBase;
   UINT64                MemorySize;
@@ -414,6 +418,80 @@ PublishPeiMemory (
     if (MemorySize > PeiMemoryCap) {
       MemoryBase = LowerMemorySize - PeiMemoryCap;
       MemorySize = PeiMemoryCap;
+    }
+  }
+
+  //
+  // Bhyve uses an E820 table to reserve certain kinds of memory like Graphics
+  // Stolen Memory. These reserved memory regions could overlap with the PEI
+  // core memory which leads to undefined behaviour. Check the E820 table for
+  // a memory region starting at or below MemoryBase which is equal or larger
+  // than MemorySize.
+  //
+  if (!EFI_ERROR (QemuFwCfgFindFile ("etc/e820", &Item, &ItemSize))) {
+    //
+    // Set a new base address based on E820 table.
+    //
+
+    EFI_PHYSICAL_ADDRESS  MaxAddress;
+    UINT64                EntryEnd;
+    UINT64                EndAddr;
+
+    if (ItemSize % sizeof (E820Entry) != 0) {
+      DEBUG ((DEBUG_INFO, "%a: invalid E820 size\n", __FUNCTION__));
+      return EFI_PROTOCOL_ERROR;
+    }
+
+    QemuFwCfgSelectItem (Item);
+
+    MaxAddress = MemoryBase + MemorySize;
+    MemoryBase = 0;
+
+    for (UINTN i = 0; i < ItemSize; i += sizeof (E820Entry)) {
+      QemuFwCfgReadBytes (sizeof (E820Entry), &E820Entry);
+
+      if (E820Entry.BaseAddr > MaxAddress) {
+        //
+        // E820 is always sorted in ascending order. For that reason, BaseAddr
+        // will always be larger than MaxAddress for the following entries.
+        //
+        break;
+      } else if (E820Entry.Type != EfiAcpiAddressRangeMemory) {
+        continue;
+      }
+
+      //
+      // Since MemoryBase should be at top of the memory, calculate the end
+      // address of the entry. Additionally, MemoryBase and MemorySize are page
+      // aligned. For that reason, page align EntryEnd too.
+      //
+      EntryEnd = (E820Entry.BaseAddr + E820Entry.Length) & ~EFI_PAGE_MASK;
+      if (E820Entry.BaseAddr > EntryEnd) {
+        //
+        // Either BaseAddr + Length overflows or the entry is smaller than a
+        // page. In both cases, we can't use it.
+        //
+        continue;
+      }
+
+      EndAddr = MIN (EntryEnd, MaxAddress);
+      if (EndAddr - E820Entry.BaseAddr < MemorySize) {
+        //
+        // E820 entry is too small for the Pei core memory.
+        //
+        continue;
+      }
+
+      MemoryBase = EndAddr - MemorySize;
+    }
+
+    if (MemoryBase == 0) {
+      DEBUG ((
+        DEBUG_ERROR,
+        "%a: Unable to find suitable PeiCore address\n",
+        __FUNCTION__
+        ));
+      return EFI_OUT_OF_RESOURCES;
     }
   }
 

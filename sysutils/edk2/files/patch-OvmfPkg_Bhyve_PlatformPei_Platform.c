--- OvmfPkg/Bhyve/PlatformPei/Platform.c.orig	2023-08-25 01:54:50 UTC
+++ OvmfPkg/Bhyve/PlatformPei/Platform.c
@@ -29,6 +29,7 @@
 #include <Library/PeimEntryPoint.h>
 #include <Library/PeiServicesLib.h>
 #include <Library/PlatformInitLib.h>
+#include <Library/QemuFwCfgLib.h>
 #include <Library/ResourcePublicationLib.h>
 #include <Guid/MemoryTypeInformation.h>
 #include <Ppi/MasterBootMode.h>
@@ -493,6 +494,118 @@ DebugDumpCmos (
   }
 }
 
+STATIC
+EFI_STATUS
+E820ReserveMemory (
+  VOID
+  )
+{
+  EFI_E820_ENTRY64      E820Entry;
+  FIRMWARE_CONFIG_ITEM  Item;
+  UINTN                 Size;
+  EFI_STATUS            Status;
+
+  Status = QemuFwCfgFindFile ("etc/e820", &Item, &Size);
+  if (EFI_ERROR (Status)) {
+    DEBUG ((DEBUG_INFO, "%a: E820 not found: %r\n", __FUNCTION__, Status));
+    return Status;
+  }
+
+  if (Size % sizeof (E820Entry) != 0) {
+    DEBUG ((DEBUG_INFO, "%a: invalid E820 size\n", __FUNCTION__));
+    return EFI_PROTOCOL_ERROR;
+  }
+
+  QemuFwCfgSelectItem (Item);
+  for (UINTN i = 0; i < Size; i += sizeof (E820Entry)) {
+    UINT64  Base;
+    UINT64  End;
+
+    QemuFwCfgReadBytes (sizeof (E820Entry), &E820Entry);
+
+    DEBUG_CODE (
+      CHAR8 *E820TypeName;
+      switch (E820Entry.Type) {
+      case EfiAcpiAddressRangeMemory:
+        E820TypeName = "RAM     ";
+        break;
+      case EfiAcpiAddressRangeReserved:
+        E820TypeName = "Reserved";
+        break;
+      case EfiAcpiAddressRangeACPI:
+        E820TypeName = "ACPI    ";
+        break;
+      case EfiAcpiAddressRangeNVS:
+        E820TypeName = "NVS     ";
+        break;
+      default:
+        E820TypeName = "Unknown ";
+        break;
+    }
+
+      DEBUG ((
+        DEBUG_INFO,
+        "E820 entry [ %16lx, %16lx] (%a)\n",
+        E820Entry.BaseAddr,
+        E820Entry.BaseAddr + E820Entry.Length,
+        E820TypeName
+        ));
+      );
+
+    //
+    // Round down base and round up length to page boundary
+    //
+    Base = E820Entry.BaseAddr & ~(UINT64)EFI_PAGE_MASK;
+    End  = ALIGN_VALUE (
+             E820Entry.BaseAddr + E820Entry.Length,
+             (UINT64)EFI_PAGE_SIZE
+             );
+
+    switch (E820Entry.Type) {
+      case EfiAcpiAddressRangeReserved:
+        BuildMemoryAllocationHob (
+          Base,
+          End - Base,
+          EfiReservedMemoryType
+          );
+        break;
+
+      case EfiAcpiAddressRangeACPI:
+        BuildMemoryAllocationHob (
+          Base,
+          End - Base,
+          EfiACPIReclaimMemory
+          );
+        break;
+
+      case EfiAcpiAddressRangeNVS:
+        BuildMemoryAllocationHob (
+          Base,
+          End - Base,
+          EfiACPIMemoryNVS
+          );
+        break;
+
+      case EfiAcpiAddressRangeMemory:
+        //
+        // EfiAcpiAddressRangeMemory is usable. Do not build an Allocation HOB.
+        //
+        break;
+
+      default:
+        DEBUG ((
+          DEBUG_ERROR,
+          "%a: Unknown E820 type %d\n",
+          __FUNCTION__,
+          E820Entry.Type
+          ));
+        return EFI_PROTOCOL_ERROR;
+    }
+  }
+
+  return EFI_SUCCESS;
+}
+
 /**
   Fetch the number of boot CPUs from QEMU and expose it to UefiCpuPkg modules.
   Set the mMaxCpuCount variable.
@@ -570,6 +683,10 @@ InitializePlatform (
   IN CONST EFI_PEI_SERVICES     **PeiServices
   )
 {
+  EFI_STATUS            Status;
+  FIRMWARE_CONFIG_ITEM  Item;
+  UINTN                 Size;
+
   DEBUG ((DEBUG_INFO, "Platform PEIM Loaded\n"));
   BuildPlatformInfoHob ();
 
@@ -582,6 +699,16 @@ InitializePlatform (
   DisableApicTimerInterrupt ();
 
   DebugDumpCmos ();
+
+  //
+  // If an E820 table exists, Memory should be reserved by E820.
+  //
+  if (!EFI_ERROR (QemuFwCfgFindFile ("etc/e820", &Item, &Size))) {
+    Status = E820ReserveMemory ();
+    if (EFI_ERROR (Status)) {
+      return Status;
+    }
+  }
 
   BootModeInitialization ();
   AddressWidthInitialization ();

--- OvmfPkg/Bhyve/BhyveX64.dsc.orig	2023-09-04 06:31:31 UTC
+++ OvmfPkg/Bhyve/BhyveX64.dsc
@@ -291,6 +291,7 @@
 !endif
   CpuExceptionHandlerLib|UefiCpuPkg/Library/CpuExceptionHandlerLib/PeiCpuExceptionHandlerLib.inf
   PcdLib|MdePkg/Library/PeiPcdLib/PeiPcdLib.inf
+  QemuFwCfgLib|OvmfPkg/Library/QemuFwCfgLib/QemuFwCfgPeiLib.inf
 
   MemEncryptSevLib|OvmfPkg/Library/BaseMemEncryptSevLib/PeiMemEncryptSevLib.inf
 
@@ -685,6 +686,7 @@
   OvmfPkg/VirtioBlkDxe/VirtioBlk.inf
   OvmfPkg/VirtioScsiDxe/VirtioScsi.inf
   OvmfPkg/VirtioRngDxe/VirtioRng.inf
+  OvmfPkg/PlatformGopPolicy/PlatformGopPolicy.inf
   MdeModulePkg/Universal/WatchdogTimerDxe/WatchdogTimer.inf
   MdeModulePkg/Universal/MonotonicCounterRuntimeDxe/MonotonicCounterRuntimeDxe.inf
   MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf

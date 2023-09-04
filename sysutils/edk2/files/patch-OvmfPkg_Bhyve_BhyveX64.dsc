--- OvmfPkg/Bhyve/BhyveX64.dsc.orig	2023-09-04 06:31:31 UTC
+++ OvmfPkg/Bhyve/BhyveX64.dsc
@@ -291,6 +291,7 @@
 !endif
   CpuExceptionHandlerLib|UefiCpuPkg/Library/CpuExceptionHandlerLib/PeiCpuExceptionHandlerLib.inf
   PcdLib|MdePkg/Library/PeiPcdLib/PeiPcdLib.inf
+  QemuFwCfgLib|OvmfPkg/Library/QemuFwCfgLib/QemuFwCfgPeiLib.inf
 
   MemEncryptSevLib|OvmfPkg/Library/BaseMemEncryptSevLib/PeiMemEncryptSevLib.inf
 

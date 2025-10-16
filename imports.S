.set noreorder

#include "pspimport.s"

IMPORT_START "sceCtrl_driver",0x00010000
IMPORT_FUNC	"sceCtrl_driver",0x2BA616AF,sceCtrlPeekBufferPositive
IMPORT_FUNC	"sceCtrl_driver",0xE6085C33,sceCtrlPeekBufferNegative
IMPORT_FUNC	"sceCtrl_driver",0xBE30CED0,sceCtrlReadBufferPositive
IMPORT_FUNC	"sceCtrl_driver",0x3A6A612A,sceCtrlReadBufferNegative
IMPORT_FUNC	"sceCtrl_driver",0xD4692E77,sceCtrlPeekBufferPositive2
IMPORT_FUNC	"sceCtrl_driver",0x41BCD9ED,sceCtrlPeekBufferNegative2
IMPORT_FUNC	"sceCtrl_driver",0x3BD76EDE,sceCtrlReadBufferPositive2
IMPORT_FUNC	"sceCtrl_driver",0x7ABDEBAA,sceCtrlReadBufferNegative2
IMPORT_FUNC	"sceCtrl_driver",0x5130DAE3,sceCtrlSetButtonEmulation
IMPORT_FUNC	"sceCtrl_driver",0xDB76878D,sceCtrlSetAnalogEmulation
IMPORT_FUNC	"sceCtrl_driver",0xF8EC18BD,sceCtrlGetSamplingMode
IMPORT_FUNC	"sceCtrl_driver",0xF6E94EA3,sceCtrlSetSamplingMode
IMPORT_FUNC	"sceCtrl_driver",0xE467BEC8,sceCtrl_driver_E467BEC8
IMPORT_FUNC	"sceCtrl_driver",0x6C86AF22,sceCtrl_driver_6C86AF22

IMPORT_START "SystemCtrlForKernel",0x00090000
IMPORT_FUNC "SystemCtrlForKernel",0x159AF5CC,sctrlHENFindFunction
IMPORT_FUNC "SystemCtrlForKernel",0x826668E9,sctrlHENPatchSyscall

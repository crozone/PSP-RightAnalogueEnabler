// PSP-RightAnalogueEnabler
// .prx plugin that patches controller read functions to include right analogue stick data.
//
// Right analogue data is read from both external controller ports and combined:
// * SCE_CTRL_PORT_DS3
// * SCE_CTRL_PORT_UNKNOWN_2
//
// Ryan Crosby 2025

// We use our own sceCtrl_driver imports with imports.S
// Don't import <pspctrl.h> or link the pspctrl module or it will conflict!
#include "ctrl_imports.h"
#include "systemctrl_imports.h"

#ifdef DEBUG
#include <pspdebug.h>
#include <pspdisplay.h>
#endif

#include <pspsdk.h>
#include <psptypes.h>
#include <pspkerror.h>
#include <pspkerneltypes.h>
#include <pspthreadman.h>

#include <stdbool.h>
#include <inttypes.h>


#define str(s) #s // For stringizing defines
#define xstr(s) str(s)

#ifdef DEBUG
#define DEBUG_PRINT(...) pspDebugScreenKprintf( __VA_ARGS__ )
#else
#define DEBUG_PRINT(...) do{ } while ( 0 )
#endif

#define MODULE_NAME "RightAnalogueEnabler"
#define MAJOR_VER 1
#define MINOR_VER 0

#define MODULE_OK       0
#define MODULE_ERROR    1

// https://github.com/uofw/uofw/blob/7ca6ba13966a38667fa7c5c30a428ccd248186cf/include/common/errors.h
#define SCE_ERROR_OK                                0x0
#define SCE_ERROR_BUSY                              0x80000021

//
// PSP SDK
//
// We are building a kernel mode prx plugin
PSP_MODULE_INFO(MODULE_NAME, PSP_MODULE_KERNEL, MAJOR_VER, MINOR_VER);

// We don't allocate any heap memory, so set this to 0.
PSP_HEAP_SIZE_KB(0);
//PSP_MAIN_THREAD_ATTR(0);
//PSP_MAIN_THREAD_NAME(MODULE_NAME);

// We don't need a main thread since we only do basic setup during module start and won't stall module loading.
// This will make us be called from the module loader thread directly, instead of a secondary kernel thread.
PSP_NO_CREATE_MAIN_THREAD();

// We don't need any of the newlib features since we're not calling into stdio or stdlib etc
PSP_DISABLE_NEWLIB();

//
// Forward declarations
//
int module_start(SceSize args, void *argp);
int module_stop(SceSize args, void *argp);

static void read_right_analogue_data(SceCtrlData *pData, u8 nBufs, bool positive);
static void patch_read_functions(void);
static void restore_read_functions(void);
static s32 sceCtrlPeekBufferPositive_patch(SceCtrlData *pData, u8 nBufs);
static s32 sceCtrlPeekBufferNegative_patch(SceCtrlData *pData, u8 nBufs);
static s32 sceCtrlReadBufferPositive_patch(SceCtrlData *pData, u8 nBufs);
static s32 sceCtrlReadBufferNegative_patch(SceCtrlData *pData, u8 nBufs);

//
// Globals
//
static void *gp_sceCtrlPeekBufferPositive_func = NULL;
static void *gp_sceCtrlPeekBufferNegative_func = NULL;
static void *gp_sceCtrlReadBufferPositive_func = NULL;
static void *gp_sceCtrlReadBufferNegative_func = NULL;

inline static s32 pspMax(s32 a, s32 b)
{
    s32 ret;
    asm("max %0, %1, %2" : "=r" (ret) : "r" (a), "r" (b));
    return ret;
}

inline static s32 pspMin(s32 a, s32 b)
{
    s32 ret;
    asm("min %0, %1, %2" : "=r" (ret) : "r" (a), "r" (b));
    return ret;
}

inline static s32 clamp(s32 value, s32 min, s32 max) {
    return pspMin(pspMax(value, min), max);
}

static
void patch_read_functions(void) {
    gp_sceCtrlPeekBufferPositive_func = (void *)sctrlHENFindFunction("sceController_Service", "sceCtrl", 0x3A622550);
    gp_sceCtrlPeekBufferNegative_func = (void *)sctrlHENFindFunction("sceController_Service", "sceCtrl", 0xC152080A);
    gp_sceCtrlReadBufferPositive_func = (void *)sctrlHENFindFunction("sceController_Service", "sceCtrl", 0x1F803938);
    gp_sceCtrlReadBufferNegative_func = (void *)sctrlHENFindFunction("sceController_Service", "sceCtrl", 0x60B81F86);

    if(gp_sceCtrlPeekBufferPositive_func != NULL) {
        sctrlHENPatchSyscall(gp_sceCtrlPeekBufferPositive_func, sceCtrlPeekBufferPositive_patch);
    }
    else {
        DEBUG_PRINT("Could not find sceCtrl sceCtrlPeekBufferPositive");
    }

    if(gp_sceCtrlPeekBufferNegative_func != NULL) {
        sctrlHENPatchSyscall(gp_sceCtrlPeekBufferNegative_func, sceCtrlPeekBufferNegative_patch);
    }
    else {
        DEBUG_PRINT("Could not find sceCtrl sceCtrlPeekBufferNegative");
    }

    if(gp_sceCtrlReadBufferPositive_func != NULL) {
        sctrlHENPatchSyscall(gp_sceCtrlReadBufferPositive_func, sceCtrlReadBufferPositive_patch);
    }
    else {
        DEBUG_PRINT("Could not find sceCtrl sceCtrlReadBufferPositive");
    }

    if(gp_sceCtrlReadBufferNegative_func != NULL) {
        sctrlHENPatchSyscall(gp_sceCtrlReadBufferNegative_func, sceCtrlReadBufferNegative_patch);
    }
    else {
        DEBUG_PRINT("Could not find sceCtrl sceCtrlReadBufferNegative");
    }
}

static
void restore_read_functions(void) {
    if(gp_sceCtrlPeekBufferPositive_func != NULL) {
        sctrlHENPatchSyscall(sceCtrlPeekBufferPositive_patch, gp_sceCtrlPeekBufferPositive_func);
    }
    if(gp_sceCtrlPeekBufferNegative_func != NULL) {
        sctrlHENPatchSyscall(sceCtrlPeekBufferNegative_patch, gp_sceCtrlPeekBufferNegative_func);
    }
    if(gp_sceCtrlReadBufferPositive_func != NULL) {
        sctrlHENPatchSyscall(sceCtrlReadBufferPositive_patch, gp_sceCtrlReadBufferPositive_func);
    }
    if(gp_sceCtrlReadBufferNegative_func != NULL) {
        sctrlHENPatchSyscall(sceCtrlReadBufferNegative_patch, gp_sceCtrlReadBufferNegative_func);
    }
}

static
void read_right_analogue_data(SceCtrlData *pData, u8 nBufs, bool positive) {
    if((pData->rX | pData->rY) != 0) {
        // We already have right analogue data (maybe we're on a VITA?)
        return;
    }

    // Shift k1. sceCtrlSetSamplingMode does this internally
    // before selecting the kernel mode or user mode state.
    // We have to do this ourselves before calling sceCtrlGetSamplingMode
    // to consistently get the correct state.
    SceUInt k1 = pspSdkSetK1(pspSdkGetK1() << 11);
    u8 sampling_mode = 0;
    if((sceCtrlGetSamplingMode(&sampling_mode) != SCE_ERROR_OK)
        || (sampling_mode == SCE_CTRL_INPUT_DIGITAL_ONLY)) {
        // Analogue inputs are not enabled.
        return;
    }

    // Our controller data destination buffer is on the kernel stack, so we need to modify
    // k1 to 0 to allow sceCtrlPeekBufferPositive2 to read into the kernel memory.
    // Otherwise, it fails with SCE_ERROR_PRIV_REQUIRED.
    //SceUInt k1 = pspSdkSetK1(0);
    pspSdkSetK1(0);

    int rightX = 0;
    int rightY = 0;
    int result_ds3;
    int result_port2;
    SceCtrlData2 extData;

    // Get SCE_CTRL_PORT_DS3 port data.
    // This contains DS3 data on PSP Go, or emulated DS3 data from another plugin.
    result_ds3 = positive
        ? sceCtrlPeekBufferPositive2(SCE_CTRL_PORT_DS3, &extData, nBufs)
        : sceCtrlPeekBufferNegative2(SCE_CTRL_PORT_DS3, &extData, nBufs);
    
    if(result_ds3 > 0) {
        // SCE_CTRL_ANALOG_PAD_CENTER_VALUE is subtracted to normalize to 0 as center
        rightX += extData.rX - SCE_CTRL_ANALOG_PAD_CENTER_VALUE;
        rightY += extData.rY - SCE_CTRL_ANALOG_PAD_CENTER_VALUE;
    }

    // Get SCE_CTRL_PORT_UNKNOWN_2 port data.
    // Nothing ever officially used this controller port.
    // In future, may be used by a plugin to provide right thumbstick via serial.
    result_port2 = positive
        ? sceCtrlPeekBufferPositive2(SCE_CTRL_PORT_UNKNOWN_2, &extData, nBufs)
        : sceCtrlPeekBufferNegative2(SCE_CTRL_PORT_UNKNOWN_2, &extData, nBufs);
    
    if(result_port2 > 0) {
        rightX += extData.rX - SCE_CTRL_ANALOG_PAD_CENTER_VALUE;
        rightY += extData.rY - SCE_CTRL_ANALOG_PAD_CENTER_VALUE;
    }

    if(result_ds3 > 0 || result_port2 > 0) {
        // Shift back to 0x00-0xFF range
        rightX += SCE_CTRL_ANALOG_PAD_CENTER_VALUE;
        rightY += SCE_CTRL_ANALOG_PAD_CENTER_VALUE;

        // Clamp combined value to 0x00-0xFF
        rightX = clamp(rightX, 0x00, 0xFF);
        rightY = clamp(rightY, 0x00, 0xFF);

        pData->rX = (u8)rightX;
        pData->rY = (u8)rightY;
    }

    // Restore k1
    pspSdkSetK1(k1);
}

static
s32 sceCtrlPeekBufferPositive_patch(SceCtrlData *pData, u8 nBufs) {
    s32 result = sceCtrlPeekBufferPositive(pData, nBufs);
    if(result > 0) {
        read_right_analogue_data(pData, nBufs, true);
    }
    return result;
}

static
s32 sceCtrlPeekBufferNegative_patch(SceCtrlData *pData, u8 nBufs) {
    s32 result = sceCtrlPeekBufferNegative(pData, nBufs);
    if(result > 0) {
        read_right_analogue_data(pData, nBufs, true);
    }
    return result;
}

static
s32 sceCtrlReadBufferPositive_patch(SceCtrlData *pData, u8 nBufs) {
    s32 result = sceCtrlReadBufferPositive(pData, nBufs);
    if(result > 0) {
        read_right_analogue_data(pData, nBufs, true);
    }
    return result;
}

static
s32 sceCtrlReadBufferNegative_patch(SceCtrlData *pData, u8 nBufs) {
    s32 result = sceCtrlReadBufferNegative(pData, nBufs);
    if(result > 0) {
        read_right_analogue_data(pData, nBufs, true);
    }
    return result;
}


// Called during module init
int module_start(SceSize args, void *argp)
{
    int result;

    #ifdef DEBUG
    pspDebugScreenInit();
    #endif

    DEBUG_PRINT(MODULE_NAME " v" xstr(MAJOR_VER) "." xstr(MINOR_VER) " Module Start\n");

    DEBUG_PRINT("Patching controller functions\n");
    patch_read_functions();

    DEBUG_PRINT("Started.\n");

    return MODULE_OK;
}

// Called during module deinit
int module_stop(SceSize args, void *argp)
{
    int result;

    DEBUG_PRINT("Stopping ...\n");

    DEBUG_PRINT("Restoring controller functions\n");
    restore_read_functions();

    DEBUG_PRINT(MODULE_NAME " v" xstr(MAJOR_VER) "." xstr(MINOR_VER) " Module Stop\n");

    return MODULE_OK;
}

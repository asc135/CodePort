// ----------------------------------------------------------------------------
//  CodePort++
//
//  A Portable Operating System Abstraction Library
//  Copyright 2012 Amardeep S. Chana.  All rights reserved.
//  Use of this software is bound by the terms of the Modified BSD License.
//
//  Module Name:    cpConstants.h
//
//  Description:    Library wide constant values.
//
//  Platform:       common
//
//  History:
//  2012-08-10  asc Creation.
// ----------------------------------------------------------------------------

#ifndef CP_CONSTANTS_H
#define CP_CONSTANTS_H

namespace cp
{

extern uint8_t const k_OctetFalse;
extern uint8_t const k_OctetTrue;
extern int const k_Error;
extern int const k_InvalidDescriptor;
extern size_t const k_DefaultIoBufSize;
extern size_t const k_FloatDigits;
extern size_t const k_DoubleDigits;
extern size_t const k_MinMemBlockSize;
extern size_t const k_MinStreamBlockSize;
extern size_t const k_DefaultThreadStack;
extern size_t const k_UdpMaxMsgLen;
extern uint16_t const k_MemSentinel;
extern uint32_t const k_DefaultThreadPriority;
extern uint32_t const k_MinimumThreadPriority;

extern uint8_t const k_IpcDefaultPriority;
extern uint8_t const k_IpcMinimumPriority;
extern uint32_t const k_IpcNodeAddrMinVal;
extern uint32_t const k_IpcAccumulatorTimeout;
extern size_t const k_IpcNodeDevNameMinLen;
extern size_t const k_IpcCommsQueueDepth;
extern size_t const k_IpcAccumQueueDepth;

extern uint32_t const k_DefaultTimeout;
extern uint32_t const k_InfiniteTimeout;
extern uint32_t const k_TransmitTimeout;
extern uint32_t const k_ReceiveTimeout;
extern uint32_t const k_RequestTimeout;
extern uint32_t const k_ResponseTimeout;

extern char const k_PathSeparator[];
extern char const k_PathTempDir[];
extern char const k_BroadcastNode[];

extern uint8_t const k_ReflectLut[];

extern char const k_SerDesAuto[];
extern char const k_SerDesNative[];
extern char const k_SerDesXml[];
extern char const k_SerDesIdl[];

}   // namespace cp

#endif // CP_CONSTANTS_H

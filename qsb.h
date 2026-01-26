#pragma once

#include "structs.h"
#include <cstdint>
#include <cstring>

// QSB Contract Index - needs to be set when contract is deployed
// For now, using a placeholder that can be configured
#define QSB_CONTRACT_INDEX 19  // Update this when contract is deployed

// Procedure numbers (matching the contract)
#define QSB_PROC_LOCK 1
#define QSB_PROC_OVERRIDE_LOCK 2
#define QSB_PROC_UNLOCK 3
#define QSB_PROC_TRANSFER_ADMIN 10
#define QSB_PROC_EDIT_ORACLE_THRESHOLD 11
#define QSB_PROC_ADD_ROLE 12
#define QSB_PROC_REMOVE_ROLE 13
#define QSB_PROC_PAUSE 14
#define QSB_PROC_UNPAUSE 15
#define QSB_PROC_EDIT_FEE_PARAMETERS 16

// Structs matching the contract I/O structures
struct QSB_Order
{
    uint8_t fromAddress[32];      // sender on source chain (mirrored id)
    uint8_t toAddress[32];        // Qubic recipient when minting/unlocking
    uint64_t tokenIn;             // identifier for incoming asset
    uint64_t tokenOut;            // identifier for outgoing asset
    uint64_t amount;              // total bridged amount (Qubic units)
    uint64_t relayerFee;          // fee paid to relayer (subset of amount)
    uint32_t destinationChainId;  // e.g. Solana chain-id as used off-chain
    uint32_t networkIn;           // incoming network id
    uint32_t networkOut;          // outgoing network id
    uint32_t nonce;               // unique order nonce
};

struct QSB_OrderHash
{
    uint8_t hash[32];  // K12 digest
};

struct QSB_SignatureData
{
    uint8_t signer[32];        // oracle id (public key)
    int8_t signature[64];      // raw 64-byte signature
};

// Lock input/output
struct QSB_Lock_input
{
    uint64_t amount;
    uint64_t relayerFee;
    uint8_t toAddress[64];      // Recipient on Solana (fixed-size buffer, zero-padded)
    uint32_t networkOut;
    uint32_t nonce;
};

struct QSB_Lock_output
{
    QSB_OrderHash orderHash;
    uint8_t success;
    uint8_t _padding[7];
};

// OverrideLock input/output
struct QSB_OverrideLock_input
{
    uint8_t toAddress[64];
    uint64_t relayerFee;
    uint32_t nonce;
};

struct QSB_OverrideLock_output
{
    QSB_OrderHash orderHash;
    uint8_t success;
    uint8_t _padding[7];
};

// Unlock input/output
struct QSB_Unlock_input
{
    QSB_Order order;
    uint32_t numSignatures;
    QSB_SignatureData signatures[64];  // QSB_MAX_ORACLES
};

struct QSB_Unlock_output
{
    QSB_OrderHash orderHash;
    uint8_t success;
    uint8_t _padding[7];
};

// TransferAdmin input/output
struct QSB_TransferAdmin_input
{
    uint8_t newAdmin[32];
};

struct QSB_TransferAdmin_output
{
    uint8_t success;
    uint8_t _padding[7];
};

// EditOracleThreshold input/output
struct QSB_EditOracleThreshold_input
{
    uint8_t newThreshold;
    uint8_t _padding[7];
};

struct QSB_EditOracleThreshold_output
{
    uint8_t oldThreshold;
    uint8_t success;
    uint8_t _padding[6];
};

// AddRole input/output
struct QSB_AddRole_input
{
    uint8_t account[32];
    uint8_t role;    // 1 = Oracle, 2 = Pauser
    uint8_t _padding[7];
};

struct QSB_AddRole_output
{
    uint8_t success;
    uint8_t _padding[7];
};

// RemoveRole input/output
struct QSB_RemoveRole_input
{
    uint8_t account[32];
    uint8_t role;    // 1 = Oracle, 2 = Pauser
    uint8_t _padding[7];
};

struct QSB_RemoveRole_output
{
    uint8_t success;
    uint8_t _padding[7];
};

// Pause/Unpause input/output
struct QSB_Pause_input
{
    uint8_t _padding[8];
};

struct QSB_Pause_output
{
    uint8_t success;
    uint8_t _padding[7];
};

// EditFeeParameters input/output
struct QSB_EditFeeParameters_input
{
    uint8_t protocolFeeRecipient[32];  // updated when not zero-id
    uint8_t oracleFeeRecipient[32];     // updated when not zero-id
    uint32_t bpsFee;                    // basis points fee (0..10000)
    uint32_t protocolFee;               // share of BPS fee for protocol (0..100)
};

struct QSB_EditFeeParameters_output
{
    uint8_t success;
    uint8_t _padding[7];
};

// Function declarations
void qsbLock(const char* nodeIp, int nodePort, const char* seed,
    uint64_t amount, uint64_t relayerFee, const char* toAddressHex,
    uint32_t networkOut, uint32_t nonce, uint32_t scheduledTickOffset);

void qsbOverrideLock(const char* nodeIp, int nodePort, const char* seed,
    uint32_t nonce, const char* toAddressHex, uint64_t relayerFee,
    uint32_t scheduledTickOffset);

void qsbUnlock(const char* nodeIp, int nodePort, const char* seed,
    const QSB_Order* order, uint32_t numSignatures,
    const QSB_SignatureData* signatures, uint32_t scheduledTickOffset);

void qsbTransferAdmin(const char* nodeIp, int nodePort, const char* seed,
    const char* newAdminIdentity, uint32_t scheduledTickOffset);

void qsbEditOracleThreshold(const char* nodeIp, int nodePort, const char* seed,
    uint8_t newThreshold, uint32_t scheduledTickOffset);

void qsbAddRole(const char* nodeIp, int nodePort, const char* seed,
    const char* accountIdentity, uint8_t role, uint32_t scheduledTickOffset);

void qsbRemoveRole(const char* nodeIp, int nodePort, const char* seed,
    const char* accountIdentity, uint8_t role, uint32_t scheduledTickOffset);

void qsbPause(const char* nodeIp, int nodePort, const char* seed,
    uint32_t scheduledTickOffset);

void qsbUnpause(const char* nodeIp, int nodePort, const char* seed,
    uint32_t scheduledTickOffset);

void qsbEditFeeParameters(const char* nodeIp, int nodePort, const char* seed,
    const char* protocolFeeRecipientIdentity, const char* oracleFeeRecipientIdentity,
    uint32_t bpsFee, uint32_t protocolFee, uint32_t scheduledTickOffset);

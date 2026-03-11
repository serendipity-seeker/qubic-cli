#pragma once

#include "structs.h"
#include <cstdint>
#include <cstring>

// QSB Contract Index - needs to be set when contract is deployed
// For now, using a placeholder that can be configured
#define QSB_CONTRACT_INDEX 24  // Update this when contract is deployed

// Procedure numbers (matching the contract)
#define QSB_PROC_LOCK 1
#define QSB_PROC_OVERRIDE_LOCK 2
#define QSB_PROC_UNLOCK 3
#define QSB_PROC_CANCEL_LOCK 4
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

// CancelLock input/output (procedure 4: cancel own locked order, refund)
struct QSB_CancelLock_input
{
    uint32_t nonce;
};

struct QSB_CancelLock_output
{
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

// ---------------------------------------------------------------------------
// View function structs
// ---------------------------------------------------------------------------

// GetConfig()
struct QSB_GetConfig_output
{
    uint8_t admin[32];
    uint8_t protocolFeeRecipient[32];
    uint8_t oracleFeeRecipient[32];
    uint32_t bpsFee;
    uint32_t protocolFee;
    uint32_t oracleCount;
    uint32_t pauserCount;
    uint8_t oracleThreshold;
    uint8_t paused;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

// IsOracle() / IsPauser()
struct QSB_IsRole_input
{
    uint8_t account[32];
};

struct QSB_IsOracle_output
{
    uint8_t isOracle;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct QSB_IsPauser_output
{
    uint8_t isPauser;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

// LockedOrderEntry used by GetLockedOrder() and GetLockedOrders()
struct QSB_LockedOrderEntry
{
    uint8_t sender[32];
    uint64_t amount;
    uint64_t relayerFee;
    uint32_t networkOut;
    uint32_t nonce;
    uint8_t toAddress[64];
    QSB_OrderHash orderHash;
    uint32_t lockEpoch;
    uint8_t active;
    uint8_t _padding[3];
};

struct QSB_GetLockedOrder_input
{
    uint32_t nonce;
};

struct QSB_GetLockedOrder_output
{
    uint8_t exists;
    uint8_t _padding[7];
    QSB_LockedOrderEntry order;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

// IsOrderFilled()
struct QSB_IsOrderFilled_input
{
    QSB_OrderHash hash;
};

struct QSB_IsOrderFilled_output
{
    uint8_t filled;
    uint8_t _padding[7];

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

// ComputeOrderHash() - input: Order, output: OrderHash
struct QSB_ComputeOrderHash_input
{
    QSB_Order order;
};

struct QSB_ComputeOrderHash_output
{
    QSB_OrderHash hash;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

// GetOracles() - bulk enumeration
struct QSB_GetOracles_output
{
    uint32_t count;
    uint8_t accounts[64][32];  // QSB_MAX_ORACLES * 32 bytes

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

// GetPausers() - bulk enumeration
struct QSB_GetPausers_output
{
    uint32_t count;
    uint8_t accounts[32][32];  // QSB_MAX_PAUSERS * 32 bytes

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

// GetLockedOrders() - paginated
#define QSB_QUERY_MAX_PAGE_SIZE 64

struct QSB_GetLockedOrders_input
{
    uint32_t offset;
    uint32_t limit;
};

struct QSB_GetLockedOrders_output
{
    uint32_t totalActive;
    uint32_t returned;
    QSB_LockedOrderEntry entries[QSB_QUERY_MAX_PAGE_SIZE];

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

// GetFilledOrders() - paginated
struct QSB_GetFilledOrders_input
{
    uint32_t offset;
    uint32_t limit;
};

struct QSB_GetFilledOrders_output
{
    uint32_t totalActive;
    uint32_t returned;
    QSB_OrderHash hashes[QSB_QUERY_MAX_PAGE_SIZE];

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

// Function declarations
void qsbLock(const char* nodeIp, int nodePort, const char* seed,
    uint64_t amount, uint64_t relayerFee, const char* toAddressHex,
    uint32_t networkOut, uint32_t nonce, uint32_t scheduledTickOffset);

void qsbOverrideLock(const char* nodeIp, int nodePort, const char* seed,
    uint32_t nonce, const char* toAddressHex, uint64_t relayerFee,
    uint32_t scheduledTickOffset);

void qsbCancelLock(const char* nodeIp, int nodePort, const char* seed,
    uint32_t nonce, uint32_t scheduledTickOffset);

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

// View / helper functions
void qsbGetConfig(const char* nodeIp, int nodePort);
void qsbIsOracle(const char* nodeIp, int nodePort, const char* accountIdentity);
void qsbIsPauser(const char* nodeIp, int nodePort, const char* accountIdentity);
void qsbGetLockedOrder(const char* nodeIp, int nodePort, uint32_t nonce);
void qsbIsOrderFilled(const char* nodeIp, int nodePort, const char* orderHashHex);
void qsbComputeOrderHash(const char* nodeIp, int nodePort,
    const char* fromIdentity, const char* toIdentity,
    uint64_t amount, uint64_t relayerFee, uint32_t destinationChainId,
    uint32_t networkIn, uint32_t networkOut, uint32_t nonce);
void qsbGetOracles(const char* nodeIp, int nodePort);
void qsbGetPausers(const char* nodeIp, int nodePort);
void qsbGetLockedOrders(const char* nodeIp, int nodePort, uint32_t offset, uint32_t limit);
void qsbGetFilledOrders(const char* nodeIp, int nodePort, uint32_t offset, uint32_t limit);

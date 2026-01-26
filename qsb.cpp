#include "qsb.h"
#include "wallet_utils.h"
#include "key_utils.h"
#include "sanity_check.h"
#include <cstring>
#include <iostream>
#include <iomanip>

// Helper function to convert hex string to bytes
static bool hexStringToBytes(const char* hexStr, uint8_t* output, size_t outputSize)
{
    if (!hexStr || strlen(hexStr) != outputSize * 2)
    {
        return false;
    }

    for (size_t i = 0; i < outputSize; ++i)
    {
        char c1 = hexStr[i * 2];
        char c2 = hexStr[i * 2 + 1];
        
        uint8_t byte = 0;
        if (c1 >= '0' && c1 <= '9')
            byte |= (c1 - '0') << 4;
        else if (c1 >= 'a' && c1 <= 'f')
            byte |= (c1 - 'a' + 10) << 4;
        else if (c1 >= 'A' && c1 <= 'F')
            byte |= (c1 - 'A' + 10) << 4;
        else
            return false;

        if (c2 >= '0' && c2 <= '9')
            byte |= (c2 - '0');
        else if (c2 >= 'a' && c2 <= 'f')
            byte |= (c2 - 'a' + 10);
        else if (c2 >= 'A' && c2 <= 'F')
            byte |= (c2 - 'A' + 10);
        else
            return false;

        output[i] = byte;
    }
    return true;
}

// Helper function to print order hash
static void printOrderHash(const QSB_OrderHash& hash)
{
    std::cout << "Order Hash: ";
    for (size_t i = 0; i < 32; ++i)
    {
        std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)hash.hash[i];
    }
    std::cout << std::dec << std::endl;
}

void qsbLock(const char* nodeIp, int nodePort, const char* seed,
    uint64_t amount, uint64_t relayerFee, const char* toAddressHex,
    uint32_t networkOut, uint32_t nonce, uint32_t scheduledTickOffset)
{
    QSB_Lock_input input;
    memset(&input, 0, sizeof(input));
    
    input.amount = amount;
    input.relayerFee = relayerFee;
    input.networkOut = networkOut;
    input.nonce = nonce;

    // Convert hex string to bytes for toAddress
    if (toAddressHex && strlen(toAddressHex) > 0)
    {
        if (!hexStringToBytes(toAddressHex, input.toAddress, 64))
        {
            std::cout << "ERROR: Invalid toAddress hex string. Must be 128 hex characters (64 bytes)." << std::endl;
            return;
        }
    }

    std::cout << "Locking " << amount << " QU (relayer fee: " << relayerFee 
              << ", network: " << networkOut << ", nonce: " << nonce << ")..." << std::endl;

    makeContractTransaction(nodeIp, nodePort, seed,
        QSB_CONTRACT_INDEX, QSB_PROC_LOCK, amount,
        sizeof(input), &input, scheduledTickOffset);
}

void qsbOverrideLock(const char* nodeIp, int nodePort, const char* seed,
    uint32_t nonce, const char* toAddressHex, uint64_t relayerFee,
    uint32_t scheduledTickOffset)
{
    QSB_OverrideLock_input input;
    memset(&input, 0, sizeof(input));
    
    input.nonce = nonce;
    input.relayerFee = relayerFee;

    // Convert hex string to bytes for toAddress
    if (toAddressHex && strlen(toAddressHex) > 0)
    {
        if (!hexStringToBytes(toAddressHex, input.toAddress, 64))
        {
            std::cout << "ERROR: Invalid toAddress hex string. Must be 128 hex characters (64 bytes)." << std::endl;
            return;
        }
    }

    std::cout << "Overriding lock for nonce " << nonce 
              << " (relayer fee: " << relayerFee << ")..." << std::endl;

    makeContractTransaction(nodeIp, nodePort, seed,
        QSB_CONTRACT_INDEX, QSB_PROC_OVERRIDE_LOCK, 0,
        sizeof(input), &input, scheduledTickOffset);
}

void qsbUnlock(const char* nodeIp, int nodePort, const char* seed,
    const QSB_Order* order, uint32_t numSignatures,
    const QSB_SignatureData* signatures, uint32_t scheduledTickOffset)
{
    if (numSignatures > 64)
    {
        std::cout << "ERROR: Too many signatures (max 64)." << std::endl;
        return;
    }

    QSB_Unlock_input input;
    memset(&input, 0, sizeof(input));
    
    memcpy(&input.order, order, sizeof(QSB_Order));
    input.numSignatures = numSignatures;
    
    for (uint32_t i = 0; i < numSignatures && i < 64; ++i)
    {
        memcpy(&input.signatures[i], &signatures[i], sizeof(QSB_SignatureData));
    }

    std::cout << "Unlocking order with " << numSignatures << " signature(s)..." << std::endl;

    makeContractTransaction(nodeIp, nodePort, seed,
        QSB_CONTRACT_INDEX, QSB_PROC_UNLOCK, 0,
        sizeof(input), &input, scheduledTickOffset);
}

void qsbTransferAdmin(const char* nodeIp, int nodePort, const char* seed,
    const char* newAdminIdentity, uint32_t scheduledTickOffset)
{
    QSB_TransferAdmin_input input;
    memset(&input, 0, sizeof(input));
    
    sanityCheckIdentity(newAdminIdentity);
    getPublicKeyFromIdentity(newAdminIdentity, input.newAdmin);

    std::cout << "Transferring admin to " << newAdminIdentity << "..." << std::endl;

    makeContractTransaction(nodeIp, nodePort, seed,
        QSB_CONTRACT_INDEX, QSB_PROC_TRANSFER_ADMIN, 0,
        sizeof(input), &input, scheduledTickOffset);
}

void qsbEditOracleThreshold(const char* nodeIp, int nodePort, const char* seed,
    uint8_t newThreshold, uint32_t scheduledTickOffset)
{
    if (newThreshold == 0 || newThreshold > 100)
    {
        std::cout << "ERROR: Oracle threshold must be between 1 and 100." << std::endl;
        return;
    }

    QSB_EditOracleThreshold_input input;
    memset(&input, 0, sizeof(input));
    input.newThreshold = newThreshold;

    std::cout << "Setting oracle threshold to " << (int)newThreshold << "%..." << std::endl;

    makeContractTransaction(nodeIp, nodePort, seed,
        QSB_CONTRACT_INDEX, QSB_PROC_EDIT_ORACLE_THRESHOLD, 0,
        sizeof(input), &input, scheduledTickOffset);
}

void qsbAddRole(const char* nodeIp, int nodePort, const char* seed,
    const char* accountIdentity, uint8_t role, uint32_t scheduledTickOffset)
{
    if (role != 1 && role != 2)
    {
        std::cout << "ERROR: Role must be 1 (Oracle) or 2 (Pauser)." << std::endl;
        return;
    }

    QSB_AddRole_input input;
    memset(&input, 0, sizeof(input));
    
    sanityCheckIdentity(accountIdentity);
    getPublicKeyFromIdentity(accountIdentity, input.account);
    input.role = role;

    const char* roleName = (role == 1) ? "Oracle" : "Pauser";
    std::cout << "Adding " << roleName << " role to " << accountIdentity << "..." << std::endl;

    makeContractTransaction(nodeIp, nodePort, seed,
        QSB_CONTRACT_INDEX, QSB_PROC_ADD_ROLE, 0,
        sizeof(input), &input, scheduledTickOffset);
}

void qsbRemoveRole(const char* nodeIp, int nodePort, const char* seed,
    const char* accountIdentity, uint8_t role, uint32_t scheduledTickOffset)
{
    if (role != 1 && role != 2)
    {
        std::cout << "ERROR: Role must be 1 (Oracle) or 2 (Pauser)." << std::endl;
        return;
    }

    QSB_RemoveRole_input input;
    memset(&input, 0, sizeof(input));
    
    sanityCheckIdentity(accountIdentity);
    getPublicKeyFromIdentity(accountIdentity, input.account);
    input.role = role;

    const char* roleName = (role == 1) ? "Oracle" : "Pauser";
    std::cout << "Removing " << roleName << " role from " << accountIdentity << "..." << std::endl;

    makeContractTransaction(nodeIp, nodePort, seed,
        QSB_CONTRACT_INDEX, QSB_PROC_REMOVE_ROLE, 0,
        sizeof(input), &input, scheduledTickOffset);
}

void qsbPause(const char* nodeIp, int nodePort, const char* seed,
    uint32_t scheduledTickOffset)
{
    QSB_Pause_input input;
    memset(&input, 0, sizeof(input));

    std::cout << "Pausing QSB contract..." << std::endl;

    makeContractTransaction(nodeIp, nodePort, seed,
        QSB_CONTRACT_INDEX, QSB_PROC_PAUSE, 0,
        sizeof(input), &input, scheduledTickOffset);
}

void qsbUnpause(const char* nodeIp, int nodePort, const char* seed,
    uint32_t scheduledTickOffset)
{
    QSB_Pause_input input;  // Unpause uses same input structure
    memset(&input, 0, sizeof(input));

    std::cout << "Unpausing QSB contract..." << std::endl;

    makeContractTransaction(nodeIp, nodePort, seed,
        QSB_CONTRACT_INDEX, QSB_PROC_UNPAUSE, 0,
        sizeof(input), &input, scheduledTickOffset);
}

void qsbEditFeeParameters(const char* nodeIp, int nodePort, const char* seed,
    const char* protocolFeeRecipientIdentity, const char* oracleFeeRecipientIdentity,
    uint32_t bpsFee, uint32_t protocolFee, uint32_t scheduledTickOffset)
{
    QSB_EditFeeParameters_input input;
    memset(&input, 0, sizeof(input));
    
    input.bpsFee = bpsFee;
    input.protocolFee = protocolFee;

    // Set protocol fee recipient if provided
    if (protocolFeeRecipientIdentity && strlen(protocolFeeRecipientIdentity) > 0)
    {
        sanityCheckIdentity(protocolFeeRecipientIdentity);
        getPublicKeyFromIdentity(protocolFeeRecipientIdentity, input.protocolFeeRecipient);
    }

    // Set oracle fee recipient if provided
    if (oracleFeeRecipientIdentity && strlen(oracleFeeRecipientIdentity) > 0)
    {
        sanityCheckIdentity(oracleFeeRecipientIdentity);
        getPublicKeyFromIdentity(oracleFeeRecipientIdentity, input.oracleFeeRecipient);
    }

    std::cout << "Editing fee parameters (BPS fee: " << bpsFee 
              << ", Protocol fee: " << protocolFee << "%)..." << std::endl;

    makeContractTransaction(nodeIp, nodePort, seed,
        QSB_CONTRACT_INDEX, QSB_PROC_EDIT_FEE_PARAMETERS, 0,
        sizeof(input), &input, scheduledTickOffset);
}

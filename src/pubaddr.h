
// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2014 The BitcoinDark developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef _BITCOINPUBADDR_H_
#define _BITCOINPUBADDR_H_ 1

#include <set>
#include <string>

#include "uint256.h"
#include "util.h"

class CNode;

/** PubAddrs are for broadcasting SuperNET API information to connected peers.
 * PubAddr messages are broadcast as a vector of signed data.  Unserializing may
 * not read the entire buffer if the pubaddr is for a newer version, but older
 * versions can still relay the original data.
 */
class CUnsignedPubAddr
{
public:
    int nVersion;
    int64_t nRelayUntil;      // when newer nodes stop relaying to newer nodes
    int64_t nExpiration;
    int nID;                //nodeID
    int nMinVer;            // lowest version inclusive
    int nMaxVer;            // highest version inclusive
    std::set<std::string> setSubVer;  // empty matches all
    int nPriority;

    std::string teleportMsg;

    IMPLEMENT_SERIALIZE
    (
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(nRelayUntil);
        READWRITE(nExpiration);
        READWRITE(nID);
        READWRITE(nMinVer);
        READWRITE(nMaxVer);
        READWRITE(setSubVer);
        READWRITE(nPriority);
        READWRITE(teleportMsg);
    )

    void SetNull();

    std::string ToString() const;
    void print() const;
};

/** A pubaddr is a combination of a serialized CUnsignedPubAddr and a signature. */
class CPubAddr : public CUnsignedPubAddr
{
public:
    std::vector<unsigned char> vchMsg;
    std::vector<unsigned char> vchSig;

    CPubAddr()
    {
        SetNull();
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(vchMsg);
        READWRITE(vchSig);
    )

    void SetNull();
    bool IsNull() const;
    uint256 GetHash() const;
    bool IsInEffect() const;
    bool Cancels(const CPubAddr& pubaddr) const;
    bool RelayTo(CNode* pnode) const;
    bool CheckSignature() const;
    bool ProcessPubAddr(bool fThread = true);

    /*
     * Get copy of (active) pubaddr object by hash. Returns a null pubaddr if it is not found.
     */
    static CPubAddr getPubAddrByHash(const uint256 &hash);
};

#endif

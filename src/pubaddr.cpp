
//
// PubAddr system for sending Teleport messages
//

#include <algorithm>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/foreach.hpp>
#include <map>

#include "pubaddr.h"
#include "key.h"
#include "net.h"
#include "sync.h"
#include "ui_interface.h"

using namespace std;

map<uint256, CPubAddr> mapPubAddrs;
CCriticalSection cs_mapPubAddrs;

static const char* pszMainKey = "0486bce1bac0d543f104cbff2bd23680056a3b9ea05e1137d2ff90eeb5e08472eb500322593a2cb06fbf8297d7beb6cd30cb90f98153b5b7cce1493749e41e0284";

// TestNet pubaddrs pubKey
static const char* pszTestKey = "0471dc165db490094d35cde15b1f5d755fa6ad6f2b5ed0f340e3f17f57389c3c2af113a8cbcc885bde73305a553b5640c83021128008ddf882e856336269080496";

// TestNet pubaddrs private key
// "308201130201010420b665cff1884e53da26376fd1b433812c9a5a8a4d5221533b15b9629789bb7e42a081a53081a2020101302c06072a8648ce3d0101022100fffffffffffffffffffffffffffffffffffffffffffffffffffffffefffffc2f300604010004010704410479be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798483ada7726a3c4655da4fbfc0e1108a8fd17b448a68554199c47d08ffb10d4b8022100fffffffffffffffffffffffffffffffebaaedce6af48a03bbfd25e8cd0364141020101a1440342000471dc165db490094d35cde15b1f5d755fa6ad6f2b5ed0f340e3f17f57389c3c2af113a8cbcc885bde73305a553b5640c83021128008ddf882e856336269080496"

void CUnsignedPubAddr::SetNull()
{
    nVersion = 1;
    nRelayUntil = 0;
    nExpiration = 0;
    nID = 0;
    nCancel = 0;
    setCancel.clear();
    nMinVer = 0;
    nMaxVer = 0;
    nPriority = 0;

    teleportMsg.clear();
}

std::string CUnsignedPubAddr::ToString() const
{
    std::string strSetCancel;
    BOOST_FOREACH(int n, setCancel)
        strSetCancel += strprintf("%d ", n);
    std::string strSetSubVer;
    BOOST_FOREACH(std::string str, setSubVer)
        strSetSubVer += "\"" + str + "\" ";
    return strprintf(
        "CPubAddr(\n"
        "    nVersion     = %d\n"
        "    nRelayUntil  = %"PRId64"\n"
        "    nExpiration  = %"PRId64"\n"
        "    nID          = %d\n"
        "    nCancel      = %d\n"
        "    setCancel    = %s\n"
        "    nMinVer      = %d\n"
        "    nMaxVer      = %d\n"
        "    setSubVer    = %s\n"
        "    nPriority    = %d\n"
        "    teleportMsg = \"%s\"\n"
        ")\n",
        nVersion,
        nRelayUntil,
        nExpiration,
        nID,
        nCancel,
        strSetCancel.c_str(),
        nMinVer,
        nMaxVer,
        strSetSubVer.c_str(),
        nPriority,
        teleportMsg.c_str());
}

void CUnsignedPubAddr::print() const
{
    printf("%s", ToString().c_str());
}

void CPubAddr::SetNull()
{
    CUnsignedPubAddr::SetNull();
    vchMsg.clear();
    vchSig.clear();
}

bool CPubAddr::IsNull() const
{
    return (nExpiration == 0);
}

uint256 CPubAddr::GetHash() const
{
    return Hash(this->vchMsg.begin(), this->vchMsg.end());
}

bool CPubAddr::IsInEffect() const
{
    return (GetAdjustedTime() < nExpiration);
}

bool CPubAddr::Cancels(const CPubAddr& pubaddr) const
{
    if (!IsInEffect())
        return false; // this was a no-op before 31403
    return (pubaddr.nID <= nCancel || setCancel.count(pubaddr.nID));
}



bool CPubAddr::RelayTo(CNode* pnode) const
{
    if (!IsInEffect())
        return false;
    // returns true if wasn't already contained in the set
    if (pnode->setPubAddrKnown.insert(GetHash()).second)
    {
        if (GetAdjustedTime() < nRelayUntil)
        {
            pnode->PushMessage("pubaddr", *this);
            return true;
        }
    }
    return false;
}

bool CPubAddr::CheckSignature() const //anyone can use teleport.
{
    // Unserialize the data
    CDataStream sMsg(vchMsg, SER_NETWORK, PROTOCOL_VERSION);
    sMsg >> *(CUnsignedPubAddr*)this;
    return true;
}

CPubAddr CPubAddr::getPubAddrByHash(const uint256 &hash)
{
    CPubAddr retval;
    {
        LOCK(cs_mapPubAddrs);
        map<uint256, CPubAddr>::iterator mi = mapPubAddrs.find(hash);
        if(mi != mapPubAddrs.end())
            retval = mi->second;
    }
    return retval;
}

bool CPubAddr::ProcessPubAddr(bool fThread)
{
    if (!CheckSignature())
        return false;
    if (!IsInEffect())
        return false;

    {
        LOCK(cs_mapPubAddrs);
        // Cancel previous pubaddrs
        for (map<uint256, CPubAddr>::iterator mi = mapPubAddrs.begin(); mi != mapPubAddrs.end();)
        {
            const CPubAddr& pubaddr = (*mi).second;
            if (Cancels(pubaddr))
            {
                printf("cancelling pubaddr %d\n", pubaddr.nID);
                mapPubAddrs.erase(mi++);
            }
            else if (!pubaddr.IsInEffect())
            {
                printf("expiring pubaddr %d\n", pubaddr.nID);
                mapPubAddrs.erase(mi++);
            }
            else
                mi++;
        }

        // Check if this pubaddr has been cancelled
        BOOST_FOREACH(PAIRTYPE(const uint256, CPubAddr)& item, mapPubAddrs)
        {
            const CPubAddr& pubaddr = item.second;
            if (pubaddr.Cancels(*this))
            {
                printf("pubaddr already cancelled by %d\n", pubaddr.nID);
                return false;
            }
        }

        // Add to mapPubAddrs
        mapPubAddrs.insert(make_pair(GetHash(), *this));

    }

    printf("accepted pubaddr %d\n", nID);
    return true;
}

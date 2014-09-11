//
//2014 BTCDDev
//
// PubAddr system for sending SuperNET API messages
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

void CUnsignedPubAddr::SetNull()
{
    nVersion = 1;
    nRelayUntil = 0;
    nExpiration = 0;
    nID = 0;
    nMinVer = 0;
    nMaxVer = 0;
    nPriority = 0;

    teleportMsg.clear();
}

std::string CUnsignedPubAddr::ToString() const
{
    std::string strSetSubVer;
    BOOST_FOREACH(std::string str, setSubVer)
        strSetSubVer += "\"" + str + "\" ";
    return strprintf(
        "CPubAddr(\n"
        "    nVersion     = %d\n"
        "    nRelayUntil  = %"PRId64"\n"
        "    nExpiration  = %"PRId64"\n"
        "    nID          = %d\n"
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

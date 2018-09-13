// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "checkpoints.h"

#include "main.h"
#include "uint256.h"

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;

    // How many times we expect transactions after the last checkpoint to
    // be slower. This number is a compromise, as it can't be accurate for
    // every system. When reindexing from a fast disk with a slow CPU, it
    // can be up to 20, while when downloading from a slow network with a
    // fast multicore CPU, it won't be much higher than 1.
    static const double fSigcheckVerificationFactor = 5.0;

    struct CCheckpointData {
        const MapCheckpoints *mapCheckpoints;
        int64 nTimeLastCheckpoint;
        int64 nTransactionsLastCheckpoint;
        double fTransactionsPerDay;
    };

    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        (     0, uint256("0xcb016c109bd77fcaa9db94f2bf7caf7d6db74646e0439d3760706d2fb47d9512"))
        (    25, uint256("0x6a1e03792d7fb1c8d6a3201fd1a9caaa3db51169723f6d3cda9438d680f584ce"))
        (    50, uint256("0x28fb8caec56a491d9427fe04a3b644ed85ba21b04cbc9a8ceca145c7b6c274b6"))
        (    75, uint256("0xa0cfd5fc18e9fded55a8095be9af519fcf9a21ada215fcfb02f87519456e817f"))
        (   100, uint256("0x6a3446a45883323b433eadde46cfb4dd0d56fbac94302db4170cec8b7455d447"))
        (   250, uint256("0x30639dcd17dff933d85388ff545546029286a563c2f8cdb83cd1ddb74d4df86e"))
        (   500, uint256("0x0bd2051da3e80080e40398a09ea88ed5b3a52e872e87b5bde78820662e557b12"))
        (   750, uint256("0xda1ddb7b9b5c4b8ed1714246f994cb64561500eeb91a88f4582164475ae5b06e"))
        (  1000, uint256("0x74442db27eb6fd5a7abd1b0e263a440754dda3e53bad5ab3af7345d9be502168"))
        (  2500, uint256("0x5d74d8a850cd5328c7ac753786dbb443829fd2477591edf7cae34dcc1a786a57"))
        (  5000, uint256("0xe8b5202a8205841ab77431341deef46fec87c1d5862c6e9aa77820192bf39502"))
        (  7500, uint256("0x4aeec0a3ac345cc83a193622372a26f85708a94634417235476459db157f0a1c"))
        ( 10000, uint256("0x8d07c486c1f259722333f425b7eb3dd088e05672b352c2f7bbf383ed86256f56"))
        ( 20000, uint256("0x80947326a70449622f0c2de2ae20373abcdb56e98853e82104d716ea4e1454bf"))
        ( 30000, uint256("0x8ec0ba418e3bc88159e65a55bd0cafc5532c5dffc54edabf5d0cee726a8468cb"))
        ( 40000, uint256("0x16501c373ced2d426d9b620f61b43f793f9ce422789d505f16b1d4c2730582b7"))
        ( 50000, uint256("0x48c2e1c478b34b5ea1e957cc3301c46556edebee5127a1e814241e07e433e392"))
        ( 60000, uint256("0xe2eeb07ca3535b00a5f63fd2aa5860682e488641e533e1b1ccf5293ef1a473d3"))
        ;

    static const CCheckpointData data = {
        &mapCheckpoints,
        1526196289, // * UNIX timestamp of last checkpoint block
        0,    // * total number of transactions between genesis and last checkpoint
                    //   (the tx=... number in the SetBestChain debug.log lines)
        1.0     // * estimated number of transactions per day after checkpoint
    };

    static MapCheckpoints mapCheckpointsTestnet =
        boost::assign::map_list_of
        (   0, uint256("0xafca7e37d42c8ac179edfdf671b86c151a537e9b045ba8e0f3a92b02b31d70c7"))
        ;
    static const CCheckpointData dataTestnet = {
        &mapCheckpointsTestnet,
        1526196273,
        0,
        1.0
    };

    const CCheckpointData &Checkpoints() {
        if (fTestNet)
            return dataTestnet;
        else
            return data;
    }

    bool CheckBlock(int nHeight, const uint256& hash)
    {
        if (!GetBoolArg("-checkpoints", true))
            return true;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    // Guess how far we are in the verification process at the given block index
    double GuessVerificationProgress(CBlockIndex *pindex) {
        if (pindex==NULL)
            return 0.0;

        int64 nNow = time(NULL);

        double fWorkBefore = 0.0; // Amount of work done before pindex
        double fWorkAfter = 0.0;  // Amount of work left after pindex (estimated)
        // Work is defined as: 1.0 per transaction before the last checkoint, and
        // fSigcheckVerificationFactor per transaction after.

        const CCheckpointData &data = Checkpoints();

        if (pindex->nChainTx <= data.nTransactionsLastCheckpoint) {
            double nCheapBefore = pindex->nChainTx;
            double nCheapAfter = data.nTransactionsLastCheckpoint - pindex->nChainTx;
            double nExpensiveAfter = (nNow - data.nTimeLastCheckpoint)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore;
            fWorkAfter = nCheapAfter + nExpensiveAfter*fSigcheckVerificationFactor;
        } else {
            double nCheapBefore = data.nTransactionsLastCheckpoint;
            double nExpensiveBefore = pindex->nChainTx - data.nTransactionsLastCheckpoint;
            double nExpensiveAfter = (nNow - pindex->nTime)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore + nExpensiveBefore*fSigcheckVerificationFactor;
            fWorkAfter = nExpensiveAfter*fSigcheckVerificationFactor;
        }

        return fWorkBefore / (fWorkBefore + fWorkAfter);
    }

    int GetTotalBlocksEstimate()
    {
        if (!GetBoolArg("-checkpoints", true))
            return 0;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        if (!GetBoolArg("-checkpoints", true))
            return NULL;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }
}

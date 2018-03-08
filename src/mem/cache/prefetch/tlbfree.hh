/*
 * Copyright (c) 2005 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Qianlong zhang
 */

/**
 * @file
 * Describes a tlbfree prefetcher.
 */

#ifndef __MEM_CACHE_PREFETCH_TLBFREE_HH__
#define __MEM_CACHE_PREFETCH_TLBFREE_HH__

#include "mem/cache/prefetch/queued.hh"
#include "params/TLBFreePrefetcher.hh"
using namespace std;
class correlation_entry {
    private:
        uint64_t ProducerPC;

        uint64_t ConsumerPC;
        std::string ConsumerDisass;
        uint32_t ConsumerDataSize; //TODO:this shouold be producer's datasize
        int32_t ConsumerOffset;
    public:
        std::vector <correlation_entry> DepList;
        correlation_entry()
        {
            ProducerPC = 0;
            ConsumerPC = 0;
            ConsumerDisass = "";
            ConsumerDataSize = 0;
        }
        correlation_entry(uint64_t pr, uint64_t cn, std::string dis, uint32_t size)
        {
            ProducerPC = pr;
            ConsumerPC = cn;
            ConsumerDisass = dis;
            ConsumerDataSize = size;
        }
        ~correlation_entry()
        {
        }

        void SetCT(uint64_t Producer, uint64_t Consumer, std::string dis, uint32_t size)
        {
            ProducerPC = Producer;

            ConsumerPC = Consumer;
            ConsumerDisass = dis;
            ConsumerDataSize = size;
            ConsumerOffset = 0;
        }
        uint64_t GetProducerPC()
        {
            return ProducerPC;
        }
        uint64_t GetConsumerPC()
        {
            return ConsumerPC;
        }
        std::string GetDisass()
        {
            return ConsumerDisass;
        }
        uint32_t GetDataSize()
        {
            return ConsumerDataSize;
        }
        int32_t  GetConsumerOffset()
        {
            return ConsumerOffset;
        }


        void SetConsumerOffset(uint32_t off)
        {
            ConsumerOffset = off;
        }
};

class potential_producer_entry{
    private:
        Addr ProducerPC;
        Addr TargetValue;
        uint32_t DataSize; //Producer data size
    public:
        potential_producer_entry()
        {
            ProducerPC = 0;
            TargetValue = 0;
            DataSize=0;
        }
        potential_producer_entry(Addr pc, Addr target_reg, uint32_t size)
        {
            ProducerPC = pc;
            TargetValue = target_reg;
            DataSize=size;
        }
        ~potential_producer_entry()
        {
        }

        Addr GetProducerPC() const
        {
            return ProducerPC;
        }
        uint64_t GetTargetValue()
        {
            return TargetValue;
        }
        uint32_t GetDataSize()
        {
            return DataSize;
        }
};



class TLBFreePrefetcher : public QueuedPrefetcher
{
  protected:
      int degree;
      int num_flattern_prefetches;

  public:
    TLBFreePrefetcher(const TLBFreePrefetcherParams *p);
    int32_t DisassGetOffset(std::string inst);

    ~TLBFreePrefetcher() {}

    void calculatePrefetch(const PacketPtr &pkt, std::vector<Addr> &addresses);
    void PushInPrefetchList(Addr current_address, Addr prefetch_adress, std::vector<Addr> &addresses, uint32_t max_prefetches);

    /* The outest vector is coreID number, inner vector index is entry number */
     map<Addr, uint64_t>  potential_producer_window ;        //ProgramCounter, TargetValue
     vector< potential_producer_entry >  tlbfree_ppw;        //ProgramCounter, TargetValue, DataSize
     vector < correlation_entry >  correlation_table;                                //correlation table

    uint32_t potential_producer_window_size;  //those param are only used to limit the size of the queue.
    uint32_t correlation_table_size;
    uint32_t correlation_table_dep_size;
    uint32_t prefetch_request_queue_size;
    uint32_t prefetch_buffer_size;
    bool stop_at_page;
    bool only_count_lds;
};

#endif // __MEM_CACHE_PREFETCH_TLBFREE_HH__

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
 * Authors: Ron Dreslinski
 */

/**
 * @file
 * Describes a tlbfree prefetcher based on template policies.
 */

#include "mem/cache/prefetch/tlbfree.hh"
#include "debug/HWPrefetch.hh"
const Addr PAGE_SIZE = 2*1024*1024;
const Addr PAGE_MASK = ~(PAGE_SIZE-1);
const uint32_t BLKSIZE = 64;

TLBFreePrefetcher::TLBFreePrefetcher(const TLBFreePrefetcherParams *p)
    : QueuedPrefetcher(p), degree(p->degree)
{
    potential_producer_window_size = p->potential_producer_window_size;
    correlation_table_size = p->correlation_table_size;
    correlation_table_dep_size = p->correlation_table_dep_size;
    prefetch_request_queue_size = p->prefetch_request_queue_size;
    stop_at_page = p->stop_at_page;
    only_count_lds = p->only_count_lds;
}

void TLBFreePrefetcher::PushInPrefetchList(Addr current_va, Addr prefetch_address, std::vector<Addr> &prefetch_list, uint32_t max_prefetches)
{
    bool address_found =false;
    if ((!stop_at_page || ((prefetch_address & PAGE_MASK) == (current_va & PAGE_MASK))) && (prefetch_list).size() < max_prefetches)
    {
        if (prefetch_address > 0x400000 && !only_count_lds && prefetch_address<0x800000000000)
        {
            // when to use my idea, we must send the actual offset of the cache block to cache,
            // not only the cache block address, cause we need the actual data to determine the next address
            //if (prefetch_address % BLKSIZE)
            //    prefetch_address = prefetch_address-(prefetch_address % BLKSIZE);
            if (prefetch_address!=current_va)
            {
                for(std::vector<Addr>::iterator it = (prefetch_list).begin(); it != (prefetch_list).end(); ++it)
                {
                    //check if this address already have been inserted
                    //if (prefetch_address == *it)
                    if ((prefetch_address-(prefetch_address % BLKSIZE)) == (*it-(*it % BLKSIZE)) )
                        address_found=true;
                }
                if (!address_found)
                {
                    (prefetch_list).push_back(prefetch_address);
                    DPRINTF(HWPrefetch, "Pushing back  current_va :0x%lx address push_back is 0x%lx After align:0x%lx\n",\
                            current_va ,\
                            prefetch_address,\
                            prefetch_address-(prefetch_address % BLKSIZE));
                }
            }
        }
    }
}
int32_t TLBFreePrefetcher::DisassGetOffset(std::string inst_disass)
{

    std::string inst_temp = inst_disass;

    // compute the offset of load instruction to get the real base reg value,
    // which will be used to compare with  other inst's target reg in PPW
    string::size_type index1 = inst_temp.find_first_of("]", 0);
    string::size_type index2 = inst_temp.find_first_of("+");
    string::size_type index3 = inst_temp.find_first_of("-");
    string::size_type temp_index;

    int32_t inst_offset=0;
    if (index1 !=string::npos )
    {
        if(!((index2==string::npos) && (index3==string::npos)))
        {
            if (index2 != string::npos)
                temp_index=index2;
            else
                temp_index=index3;

            std::string sub_str = inst_temp.substr(temp_index+1, index1 - temp_index-1);
            DPRINTF(HWPrefetch, "sub_str: %s\n", (sub_str));
            stringstream offset(sub_str);
            offset>>hex>>inst_offset;

            if(index3!=string::npos)
                inst_offset=-inst_offset;
            DPRINTF(HWPrefetch, "inst_offset: %d\n", inst_offset);
        }
    }
    return inst_offset;
}

void
TLBFreePrefetcher::calculatePrefetch(const PacketPtr &pkt,
        std::vector<Addr> &addresses)
{
    if(pkt->req->isSplited() && !pkt->req->isFirstSplited())
    {
        // we do not handle second splited_req
        // we only handle not splited req or splited_first_req
        DPRINTF(HWPrefetch, "This is a second splited req,PC :0x%lx, cachedDisassembly: %s, pkt->size: %d\n", pkt->req->getPC(), (pkt->req->getStaticInst()?pkt->req->getStaticInst()->disassemble(pkt->req->getPC(),0):"Empty"), pkt->req->getSize());
        return;
    }
    else if(pkt->req->isSplited())
    {
        DPRINTF(HWPrefetch, "This is a first splited req,PC :0x%lx, cachedDisassembly: %s, real pkt->size: %d, size before splited is %d\n", pkt->req->getPC(), (pkt->req->getStaticInst()?pkt->req->getStaticInst()->disassemble(pkt->req->getPC(), 0):"Empty"), pkt->req->getSize(), pkt->req->getSizeBeforeSplited());
    }
    else if(!pkt->req->isSplited())
    {
        DPRINTF(HWPrefetch, "This is NOT a splited req,PC :0x%lx, real pkt->size: %d, size before splited is %d\n", pkt->req->getPC(), pkt->req->getSize(), pkt->req->getSizeBeforeSplited());
    }
    int32_t ppw_found=false;
	Addr PR = 0;
    bool already_in_ppw = false;
    bool already_in_ct = false;
    bool already_in_dep_ct = false;
    //Addr current_va = pkt->req->getVaddr();
    Addr current_va = pkt->getAddr();
    Addr current_pa = pkt->req->getPaddr();
    Addr offset = pkt->getOffset(BLKSIZE);
    unsigned data_size = pkt->req->getSizeBeforeSplited();
    DPRINTF(HWPrefetch, "Data size is:%d \n",data_size);
    uint64_t target_reg = 0;
    char * data_buf = (char*)malloc(BLKSIZE);
    memset(data_buf,0,BLKSIZE);




    //pkt=0 means memory access is send by doPrefetch(), so we not prefetch for them again
    //dir=0 means read
    if (pkt!=0  && pkt->isRead() && pkt->req->getStaticInst() && (!pkt->req->isSplited() || pkt->req->isFirstSplited()))
    {
        //DPRINTF(HWPrefetch, "This is NOT a splited req,PC :0x%lx, cachedDisassembly: %s, real pkt->size: %d, size before splited is %d\n", pkt->req->getPC(), pkt->req->getStaticInst()->disassemble(pkt->req->getPC(), 0), pkt->req->getSize(), pkt->req->getSizeBeforeSplited());
        //DPRINTF(HWPrefetch, "This is NOT a splited req,PC :0x%lx, real pkt->size: %d, size before splited is %d\n", pkt->req->getPC(), pkt->req->getSize(), pkt->req->getSizeBeforeSplited());
        {
            std::memcpy(data_buf, pkt->getConstPtr<uint8_t>(), BLKSIZE);
        }
        {
            uint64_t mem;
            switch (data_size) {
                case 1:
                    mem = pkt->get<uint8_t>();
                    break;
                case 2:
                    mem = pkt->get<uint16_t>();
                    break;
                case 4:
                    mem = pkt->get<uint32_t>();
                    break;
                case 8:
                    mem = pkt->get<uint64_t>();
                    break;
                default:
                    panic("Unhandled size in getMem.\n");
            }
            DPRINTF(HWPrefetch, "mem is 0x%lx\n", mem);
        }

        for(unsigned i = 0; i < BLKSIZE; ++i)
        {
            DPRINTF(HWPrefetch, "%02x\n", data_buf[i]);
        }
        DPRINTF(HWPrefetch, "\n");
        for(unsigned i = ((current_pa%BLKSIZE)+data_size)-1;i>=current_pa%BLKSIZE; --i)
        {
            DPRINTF(HWPrefetch, "i = %d\n", i);
            DPRINTF(HWPrefetch, "%02x\n", data_buf[i]);
            target_reg = (target_reg << 8) | reinterpret_cast<char *>(data_buf)[i];
            if (i==0)
                break;
        }
        DPRINTF(HWPrefetch, "current_va va is 0x%lx, pa is 0x%lx, offset is 0x%lx\n" , current_va, current_pa, offset);
        DPRINTF(HWPrefetch, "This inst PC is 0x%lx, cachedDisassembly is: %s, target_reg is 0x%lx\n", pkt->req->getPC(), pkt->req->getStaticInst()->disassemble(pkt->req->getPC(), 0), target_reg);
        string dis = pkt->req->getStaticInst()->disassemble(pkt->req->getPC(), 0);
        int32_t inst_offset= DisassGetOffset(pkt->req->getStaticInst()->disassemble(pkt->req->getPC(), 0));
        Addr CN = pkt->req->getPC();

        /* The outest vector is entry number */
        std::vector<potential_producer_entry>   &ppw = tlbfree_ppw;
        std::vector<potential_producer_entry> temp_ppw;
        std::vector <correlation_entry>      &ct = correlation_table;
        correlation_entry temp_ct(0,0,"",0);

        DPRINTF(HWPrefetch, "\n");
        DPRINTF(HWPrefetch, "\n");

        DPRINTF(HWPrefetch, "In function: %s,  current_va is: 0x%lx\n" ,__func__, current_va);
        //DPRINTF(HWPrefetch, "The real base reg is: 0x%lx\n", current_va+offset-inst_offset);
        DPRINTF(HWPrefetch, "STEP 1:  find producer in PPW\n");

        //step 1: find producer in PPW, the base address is the whole address(current_va+offset) - inst_offset
        for (std::vector<potential_producer_entry>::reverse_iterator it = ppw.rbegin(); it!=ppw.rend(); it++)
        {
            if(it->GetTargetValue() == current_va-inst_offset)
            {
                ppw_found=true;
                DPRINTF(HWPrefetch, "Found in PPW\n");
                //record in temp_ppw to get the current inst's producer, which is the nearest one smaller then current one
                PR = it->GetProducerPC();
                //lookup from back, the first found one is the newest, so it is the producer
                break;
            }
        }

        //step 2: hit in PPW, put PR/CN/TMPL into CT
        DPRINTF(HWPrefetch, "STEP 2:  if hit in PPW, update CT\n");
        if( ppw_found == true )
        {
            //if (pointer_loads &&!pkt->memory_info[0].dir )
            //    (*pointer_loads)++;
            //if (pointer_stores &&pkt->memory_info[0].dir )
            //    (*pointer_stores)++;

            temp_ct.SetCT(PR, CN, dis, data_size);
            temp_ct.SetConsumerOffset(inst_offset);

#ifndef INFINITE_CT
            DPRINTF(HWPrefetch, "ct.size is %d, correlation_table_size is %d\n", ct.size(), correlation_table_size);
           while (ct.size()>=correlation_table_size)
            {
               vector<correlation_entry>::iterator k=ct.begin();
               ct.erase(k);
            }
#endif
            for (std::vector<correlation_entry>::iterator iter1=ct.begin(); iter1!=ct.end(); iter1++)
            {
                if((iter1->GetProducerPC() == PR)&&(iter1->GetConsumerPC() == CN))
                {
                    already_in_ct = true;
                    DPRINTF(HWPrefetch, "In ct already have one PR is: 0x%lx, CN is 0x%lx\n", PR, CN);
                }
            }
            if (!already_in_ct &&
                    dis.find("PUSh") == string::npos &&
                    dis.find("POP") == string::npos)
            {
                ct.push_back(temp_ct);
            }
            already_in_ct = false;

            //insert to DepList
            DPRINTF(HWPrefetch, "Inserting to DepList\n");
            for (std::vector<correlation_entry>::iterator iter1=ct.begin(); iter1!=ct.end(); iter1++)
            {
                DPRINTF(HWPrefetch, "DepList size for iter1: 0x%lx, is %ld\n", iter1->GetProducerPC(), iter1->DepList.size());
                //if not empty, insert to DepList end
                if(iter1->DepList.size())
                {
                    std::vector<correlation_entry>::reverse_iterator end= iter1->DepList.rbegin();
                    DPRINTF(HWPrefetch, "end->GetProducerPC is 0x%lx, end->GetConsumerPC is 0x%lx\n", end->GetProducerPC(), end->GetConsumerPC());
                    if(end->GetConsumerPC()==PR)
                    {
                        DPRINTF(HWPrefetch, "PR and end->GetConsumerPC is 0x%lx\n", end->GetConsumerPC());
                        for (std::vector<correlation_entry>::iterator iter2=iter1->DepList.begin(); iter2!=iter1->DepList.end(); iter2++)
                        {
                            DPRINTF(HWPrefetch, "dealing with PR: 0x%lx, CN: 0x%lx\n", iter2->GetProducerPC(), iter2->GetProducerPC());
                            if((iter2->GetProducerPC() == PR)&&(iter2->GetConsumerPC() == CN))
                            {
                                already_in_dep_ct = true;
                                DPRINTF(HWPrefetch, "In dep ct already have one PR is: 0x%lx, CN is 0x%lx\n", PR, CN);
                            }
                        }
                        if (!already_in_dep_ct &&
                                dis.find("PUSh") == string::npos &&
                                dis.find("POP") == string::npos)
                        {
                            DPRINTF(HWPrefetch, "DepList pushing back: PR 0x%lx, CN 0x%lx, Diss: %s, size:%d\n", temp_ct.GetProducerPC(),temp_ct.GetConsumerPC(), (temp_ct.GetDisass()), temp_ct.GetDataSize() );
                            iter1->DepList.push_back(temp_ct);
                        }
                    }
                    already_in_dep_ct = false;
                }
                else//DepList is zero, insert the first to DepList
                {
                    if(iter1->GetConsumerPC()==PR)
                    {
                        DPRINTF(HWPrefetch, "DepList pushing back: PR 0x%lx, CN 0x%lx, Diss: %s, size:%d\n", temp_ct.GetProducerPC(),temp_ct.GetConsumerPC(), temp_ct.GetDisass(), temp_ct.GetDataSize());
                        iter1->DepList.push_back(temp_ct);
                    }
                }

#ifndef INFINITE_DEP_CT
                while (iter1->DepList.size()>=correlation_table_dep_size)
                {
                    vector<correlation_entry>::iterator j=iter1->DepList.begin();
                    iter1->DepList.erase(j);
                }
#endif
            }
            //print ct
            DPRINTF(HWPrefetch, "After insert in ct:\n");
            for (std::vector<correlation_entry>::iterator iter=ct.begin(); iter!=ct.end(); iter++)
            {
                DPRINTF(HWPrefetch, "In CT Producer is:0x%lx ConsumerPC is: 0x%lx  template is:%s, consumer offset:%d, data size is %d\n", iter->GetProducerPC(), iter->GetConsumerPC(), (iter->GetDisass()),  iter->GetConsumerOffset(), iter->GetDataSize());
                if(iter->DepList.size())
                {
                    for (std::vector<correlation_entry>::iterator iter3=iter->DepList.begin(); iter3!=iter->DepList.end(); iter3++)
                    {
                        DPRINTF(HWPrefetch, "\t\tIn DepList CT Producer is:0x%lx ConsumerPC is: 0x%lx  template is:%s, consumer offset:%d, data size is %d\n", iter3->GetProducerPC(), iter3->GetConsumerPC(), (iter3->GetDisass()), iter3->GetConsumerOffset(), iter3->GetDataSize());
                    }
                }
            }
        }

        //step 3, insert to ppw
        DPRINTF(HWPrefetch, "STEP 3:  update PPW\n");
#ifndef INFINITE_PPW
        if(ppw.size() >= potential_producer_window_size)
        {
            std::vector<potential_producer_entry>::iterator it_delete = ppw.begin();
            //TODO: ppw replacement algorithm should be updated
            DPRINTF(HWPrefetch, "PPW is full, erasing: 0x%lx\n", it_delete->GetProducerPC());;
            ppw.erase(it_delete);
        }
#endif

        potential_producer_entry ppw_entry(pkt->req->getPC(), target_reg, data_size);
        std::vector<potential_producer_entry>::iterator temp_it = ppw.begin();
        for (std::vector<potential_producer_entry>::iterator it_ppw = ppw.begin(); it_ppw!=ppw.end(); it_ppw++)
        {
            if (( it_ppw->GetProducerPC() == pkt->req->getPC()))
            {
                temp_it = it_ppw;
                already_in_ppw = true;
                DPRINTF(HWPrefetch, "In ppw already have one req->getPC(): 0x%lx  target reg is:0x%lx \n", pkt->req->getPC(), it_ppw->GetTargetValue() );
            }
        }
        if (!already_in_ppw)
        {
            if(target_reg> 0xfffff && target_reg <0x800000000000) /* if target reg is small than 0xfffff, not an base address for others, throw it*/
            {
                //the most right reg is the target reg loaded from memory
                DPRINTF(HWPrefetch, "Inserting to ppw req->getPC(): 0x%lx,   target reg is 0x%lx\n", pkt->req->getPC(),target_reg);
                ppw.push_back(ppw_entry);
            }
        }
        else
        {
            if(target_reg> 0xfffff && target_reg<0x800000000000) /* if target reg is small than 0xfffff, not an base address for others, throw it*/
            {
                DPRINTF(HWPrefetch, "updating ppw, deleting 0x%lx, target_reg is 0x%lx\n", temp_it->GetProducerPC(), temp_it->GetTargetValue());
                ppw.erase(temp_it);
                ppw.push_back(ppw_entry);
            }
        }
        //std::sort(ppw.begin(), ppw.end(), comp);
        //print ppw
        for (std::vector<potential_producer_entry>::iterator it_ppw = ppw.begin(); it_ppw!=ppw.end(); it_ppw++)
        {
            DPRINTF(HWPrefetch, "After insert in ppw, PC: 0x%lx target reg is 0x%lx\n", it_ppw->GetProducerPC(),it_ppw->GetTargetValue());
        }
        already_in_ppw=false;

        //step 4, lookup CT to get the next prefetch address
        //PC as producer to get potential consumer
        DPRINTF(HWPrefetch, "STEP 4:  get the prefetch address from CT\n");

        for (std::vector<correlation_entry>::iterator iter=ct.begin(); iter!=ct.end(); iter++)
        {
            Addr temp_temp_target_reg=0;
            if (iter->GetProducerPC() == pkt->req->getPC())
            {
                DPRINTF(HWPrefetch, "Pushing back PR is 0x%lx, CN is 0x%lx,current address is 0x%lx,  Prefetch address is 0x%lx\n", iter->GetProducerPC(), iter->GetConsumerPC(),current_va, target_reg + iter->GetConsumerOffset());
                PushInPrefetchList(current_va, target_reg + iter->GetConsumerOffset(), addresses, num_flattern_prefetches);

                //for(int32_t j = iter->GetDataSize()-1; j >= 0; --j)
                //{
                //    temp_temp_target_reg = (temp_temp_target_reg << 8) | reinterpret_cast<char *>(current_pa+offset)[j];
                //}
                DPRINTF(HWPrefetch, "target_reg is 0x%lx, temp_temp_target_reg is 0x%lx,  GetDataSize() is %d\n", target_reg, temp_temp_target_reg,  iter->GetDataSize());
            }
        }
#if 0
        /****************************************/
        /****************************************/
        /****************************************/
        //get dependency access address to prefetch
        if(degree - num_flattern_prefetches > 0)
        {
            Addr temp_target_reg=0;
            Addr last_prefetch_address=0;
            for (std::vector<correlation_entry>::iterator iter=ct.begin(); iter!=ct.end(); iter++)
            {
                //To get the offset for every comsumer inst, used to compute the prefetch address
                if ((iter->GetProducerPC() == pkt->req->getPC()) && (iter->DepList.size()>0))
                {
                    last_prefetch_address = target_reg + iter->GetConsumerOffset();
                    PushInPrefetchList(current_va+offset, last_prefetch_address, addresses, degree);

                    if ( ((last_prefetch_address & PAGE_MASK) == (current_va & PAGE_MASK)) )
                    {
                        if ((last_prefetch_address>0x600000 && last_prefetch_address<0x6fffff) || (last_prefetch_address>0x700000000000 && last_prefetch_address<0x800000000000))
                        {
                            DPRINTF(HWPrefetch, "\t\tDepList PR: 0x%lx, CN: 0x%lx, Getting data from 0x%lx, size:%d\n",iter->GetProducerPC(), iter->GetConsumerPC(), last_prefetch_address, iter->GetDataSize());
                            //for(int32_t j = iter->GetDataSize()-1; j >= 0; --j)
                            //{
                            //    temp_target_reg = (temp_target_reg << 8) | reinterpret_cast<char *>(last_prefetch_address)[j];
                            //}
                            DPRINTF(HWPrefetch, "\t\tDepList data is 0x%lx\n", temp_target_reg);
                        }
                        else
                        {
                            break;
                        }
                    }
                    for (std::vector<correlation_entry>::iterator iter4=iter->DepList.begin(); iter4!=iter->DepList.end(); iter4++)
                    {
                        PushInPrefetchList(last_prefetch_address, temp_target_reg + iter4->GetConsumerOffset(), addresses, degree);
                        last_prefetch_address = temp_target_reg + iter4->GetConsumerOffset();
                        if ( ((last_prefetch_address & PAGE_MASK) == (current_va & PAGE_MASK)) )
                        {
                            if ((last_prefetch_address>0x600000 && last_prefetch_address<0x6fffff) || (last_prefetch_address>0x700000000000 && last_prefetch_address<0x800000000000))
                            {
                                DPRINTF(HWPrefetch, "\t\tDepList PR: 0x%lx, CN: 0x%lx, Getting data from 0x%lx, size:%d\n",iter4->GetProducerPC(), iter4->GetConsumerPC(), last_prefetch_address, iter4->GetDataSize());
                                //for(int32_t j = iter4->GetDataSize()-1; j >= 0; --j)
                                //{
                                //    temp_target_reg = (temp_target_reg << 8) | reinterpret_cast<char *>(last_prefetch_address)[j];
                                //}
                                DPRINTF(HWPrefetch, "\t\tDepList data is 0x%lx\n", temp_target_reg);
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                }
            }
        }
#endif
        for(std::vector<Addr>::iterator it = addresses.begin(); it != addresses.end(); ++it)
        {
            DPRINTF(HWPrefetch, "After push back in prefetch address: 0x%lx\n", *it);
            //cout<<"After push back in prefetch address: 0x"<<hex<<*it<<endl;
        }
    }
    free(data_buf);
    DPRINTF(HWPrefetch, "Done calculatePrefetch!\n");
}

TLBFreePrefetcher*
TLBFreePrefetcherParams::create()
{
   return new TLBFreePrefetcher(this);
}

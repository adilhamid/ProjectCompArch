#ifndef RRIP_REPL_H_
#define RRIP_REPL_H_

#include "repl_policies.h"

// Static RRIP
class SRRIPReplPolicy : public ReplPolicy {
    protected:
        // add class member variables here
	uint32_t rpvMax;
	uint64_t* array;
	uint32_t numLines;
    public:
        // add member methods here, refer to repl_policies.h
		explicit SRRIPReplPolicy(uint32_t _rpvMax, uint32_t _numLines):rpvMax(_rpvMax),numLines(_numLines)
		{
			array = gm_calloc<uint64_t>(numLines);
			for (uint32_t i=0;i<numLines;i++)
			{
				array[i]=rpvMax;
			}
		}
		
		~SRRIPReplPolicy()
		{
			gm_free(array);
		}
		
		void update(uint32_t id,const MemReq* req)
		{
			if(array[id]==rpvMax)
			{
				array[id]=rpvMax-1;
			}
			else
			{
				array[id]=0;
			}
		}
		
		void replaced(uint32_t id)
		{
			array[id]=rpvMax;
		}
		
		template <typename C> inline uint32_t rank(const MemReq* req, C cands) 
		{
        	uint32_t bestCand = (uint32_t)-1;
        	for(auto ci=cands.begin();ci!=cands.end();ci.inc())
        	{
        		if(array[*ci]==rpvMax)
        		{
        			bestCand=*ci;
        		}
        	}
        	while(bestCand==(uint32_t)-1L)
        	{
        		for(auto ci=cands.begin();ci!=cands.end();ci.inc())
        		{
        			array[*ci]=array[*ci]+1;
        			if(array[*ci]==rpvMax)
        			{
        				bestCand=*ci;
        			}
        		}
        	}
        	
        	return bestCand;
        	
        }
        DECL_RANK_BINDINGS;
};
#endif // RRIP_REPL_H_

#ifndef RRIP_REPL_H_
#define RRIP_REPL_H_

#include "repl_policies.h"

 // Based on the LRU implementation in the repl_policies.h

// Static RRIP Implementation
class SRRIPReplPolicy: public ReplPolicy {
protected:
	uint32_t RRPVMaxVal; // Represents the MaxValue each entry can have
	uint64_t* array;
	uint32_t numLines;

public:
	SRRIPReplPolicy(uint32_t _numLines, uint32_t _rpvMax) :
			RRPVMaxVal(_rpvMax), numLines(_numLines){
		array = gm_calloc<uint64_t>(numLines);

		// Instiantiate the values of each array value as 3

		for( uint32_t id =0 ; id<numLines; id++){
			array[id] = RRPVMaxVal;
		}
	}

	~SRRIPReplPolicy() {
		gm_free(array);
	}

	void update(uint32_t id, const MemReq* req) {
		if (array[id] != 99999) {
			array[id] = 0; // Represents the hit case, so the value of the array is updated to 0 --> meaning the block is near-reference
		} else {
			array[id] = RRPVMaxVal-1; // Represents the replaced case, so the value of the array should be updated to 2 --> meaning the block is long distant-reference
		}
	}

	void replaced(uint32_t id) {
		array[id] = 99999; // marker to let the update function know that this block is miss case i.e replaced
	}

	template<typename C> inline uint32_t rank(const MemReq* req, C cands) {
		int bestCand = -1;

		// assumning the C cands represents the array values of the candidates i.e array[id] values in some particular order
		for (auto ci = cands.begin(); ci != cands.end(); ci.inc()) {
			if (array[*ci] == RRPVMaxVal) {
				bestCand = *ci; // returns the bestCandidate which has the value of 3 in the search
			}
		}

		while(bestCand == -1) {
			//Update the RRPV values ( increment by one )
			for (auto ci = cands.begin(); ci != cands.end(); ci.inc()) {
				array[*ci] = (array[*ci] + 1);
				if(bestCand == -1 && array[*ci] == RRPVMaxVal){ // Get the bestCand which will be the first value updated to 3 from the list of all the candidates
					bestCand = *ci ;
				}
			}

		}

		if(bestCand == -1){
			std::cout << "Adil: Serious Error happening here"<<std::endl;
		}

		return (uint32_t)bestCand;
	}

	DECL_RANK_BINDINGS;
};

#endif // RRIP_REPL_H_


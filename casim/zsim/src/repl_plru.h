/*
* repl_plru.h
*
*/

#ifndef REPL_PLRU_H_
#define REPL_PLRU_H_


#include "repl_policies.h"
#include<math.h>
#include<algorithm>
#include<iostream>
#include<vector>

using namespace std;

// PLRU Implementation
class PLRUReplPolicy : public ReplPolicy {
protected:
	uint32_t* array;
	uint32_t numLines;
	vector<uint32_t> candArray;
	vector<uint32_t> copyList;
	uint32_t numCands;

public:
	PLRUReplPolicy(uint32_t _numLines, int pNumCands) :
		numLines(_numLines), numCands(pNumCands)
	{
		info("the number of the lines are %d", numLines);
		array = gm_calloc<uint32_t>(numLines);
		for (uint32_t i = 0; i < numLines; i++) {
			array[i] = i;
		}
		candArray = vector<uint32_t>(pNumCands);
		copyList = vector<uint32_t>(pNumCands);

	}

	~PLRUReplPolicy() {
		gm_free(array);
	}
	//Populating the Candidate Array
	void populateCandArray(int startId) {
		candArray.clear();
		for (uint32_t i = 0; i < numCands; i++) {
			candArray[i] = array[startId + i];
		}
	}

	//Populate Back
	void populateArray(int startId) {
		for (uint32_t i = 0; i < numCands; i++) {
			array[startId + i] = candArray[i];
		}
	}

	// Get Updated ID
	uint32_t getUpdatedId(uint32_t id) {
		for (uint32_t t = 0; t < numCands; t++) {
			if (candArray[t] == id) {
				return t;
			}
		}
		panic("I couldn't find the ID");
	}

	void printCandArray() {
		for (uint32_t cnt = 0; cnt < numCands; cnt++) {
			info("Val ,%d", candArray[cnt]);
		}
	}

	// Update Code
	void update(uint32_t id, const MemReq* req) {

		uint32_t blockId = id / numCands;
		// Populate the CandArray
		uint32_t startId = blockId * numCands;
		populateCandArray(startId);

		if (array[blockId*numCands] == 9999999) { //case 1 : when the block is replaced; MISS
			array[id] = id;
			candArray[0] = id;
			accessUpdate(0);

		}

		else {	//case 2 : when the block is updated : HIT
			uint32_t updatedId = getUpdatedId(id);
			/*if (updatedId != id) {
				info("Found Different ID set");
				info("ID, %d", id);
				info("Update ID, %d", updatedId);

			}*/
			accessUpdate(updatedId);
			
		}
		// Populate the array back after the access update
		populateArray(blockId*numCands);
		
	}

	// replaced : check if you want to do the division and multiplication if i am getting not power of 2 as relacement for the ID
	void replaced(uint32_t id) {
		uint32_t blockId = id / numCands;
		array[blockId*numCands] = 9999999; // Marker for the MISS case
	}

	template <typename C> inline uint32_t rank(const MemReq* req, C cands) {
		auto first = cands.begin();
		// Fill candidate Array
		uint32_t bestCand = array[*first];
		return bestCand;
	}

	void accessUpdate(int location) {
		
		int length = numCands;

		copyList.clear();

		for (int i = 0; i < length; i++)
		{
			copyList[i] = candArray[i];
		}
		// now access the list in weird way
		int counterCorrected = 0;

		//Make it most away from being updated
		counterCorrected++;
		candArray[length - counterCorrected] = copyList[location];

		//Get the nearest sibling
		if (location % 2 == 0) { //Left Child
			counterCorrected++;
			candArray[length - counterCorrected] = copyList[location + 1];

		}
		else { //Right Child
			counterCorrected++;
			candArray[length - counterCorrected] = copyList[location - 1];
		}

		// General Case
		int n = log2(length);

		int lengthReducedSeq = n;
		int reducedSeq = location;
		while (counterCorrected < length) {
			reducedSeq = reducedSeq / 2;
			lengthReducedSeq--;

			if (reducedSeq % 2 == 0) {
				reducedSeq++;
			}
			else {
				reducedSeq--;
			}
			int newlocations = 0;
			int numDontCares = n - lengthReducedSeq;
			int powerDontCares = pow(2, numDontCares);
			newlocations = powerDontCares * reducedSeq;
			for (int i = powerDontCares - 1; i >= 0; i--) {
				counterCorrected++;
				candArray[length - counterCorrected] = copyList[newlocations + i];
			}
		}

	}

	DECL_RANK_BINDINGS;
};

#endif /* REPL_PLRU_H_ */
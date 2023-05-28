#include <iostream>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <stdlib.h>

using namespace std;

char helpMessageString[] =
	"---------------------------------------------------------------------------------------\n"
	"Option |   Argument            |   Meaning                              |  Default\n"
	"-h     |   none                |   prints this help text                |  none\n"
	"-k     |   number of Kibibytes |   sets the maximal size of free memory |  512\n"
	"---------------------------------------------------------------------------------------\n";

struct programArgHandler
{

	bool helpComd = false;
	int64_t kibiyts = 512;
};

struct userInput
{

	char userCommand;
	int64_t userValue;
};

struct kibYtes
{
	size_t index = 0;
	int64_t previousBlock = 1;
	int64_t nextAddress = -1;
	int64_t lastAddress = -1;
	int64_t freeSpace = -1;
};

struct alloctMem
{

	int64_t postion = 0;
	int64_t length = 0;
};

bool sortByPos (alloctMem i,alloctMem j) { 
	// sort the vector for allocted list by smallest to largest position
	return (i.postion < j.postion); 

};

// im to tired
class nextFit
{

private:
	vector<kibYtes> freeList;

	vector<alloctMem> allocList;

	int64_t headAddress;

public:
	nextFit(int64_t intialSize);

	int allocate(int64_t allocateSize);

	int freeBlock(int64_t indexAddress);

	void printList();
};

nextFit::nextFit(int64_t intialSize)
{

	freeList = vector<kibYtes>(intialSize);

	freeList[0].freeSpace = intialSize;

	for (size_t i = 0; i < freeList.size(); i++)
	{

		freeList[i].index = i;
	}

	headAddress = 0;
}

int nextFit::allocate(int64_t allocateSize)
{
	alloctMem allocatedPc;

	int64_t lastNextAddr = 0;
	int64_t leftOverSize = 0;

	int64_t pointerAllocated = 0;

	int64_t headReminder = headAddress;

	bool iterateLastNode = false;

	while (true)
	{
		//if current free block is not big enough move to next node
		if (freeList[headAddress].freeSpace >= allocateSize)
		{

			if (freeList[headAddress].nextAddress != -1)
			{// routine for list with more then one block

				//leftOverSize is for the new size of the fresh created free block
				leftOverSize = freeList[headAddress].freeSpace - allocateSize;
				lastNextAddr = freeList[headAddress].nextAddress;	//remember the next list address
				//as we will have to reset list pointers

				//set free block to be allocated block
				freeList[headAddress].freeSpace = allocateSize;

				freeList[headAddress].nextAddress = -2;

				pointerAllocated = headAddress;
				// set up allocated list block for print use
				allocatedPc.length = allocateSize;
				allocatedPc.postion = headAddress;
				allocList.push_back(allocatedPc);


				if(leftOverSize != 0){// other wise set up new free block from split
					headAddress += allocateSize;
					freeList[headAddress].freeSpace = leftOverSize;
					freeList[headAddress].nextAddress = lastNextAddr;
					freeList[lastNextAddr].lastAddress = headAddress;
					freeList[headAddress].lastAddress = -1;

				}else{
					// if consumed entire block rebuild list 
					headAddress = lastNextAddr;
					freeList[headAddress].lastAddress = -1;
				}
				break;
			}
			else
			{// routine for handling last free block in list (the const last block)

				lastNextAddr = freeList[headAddress].lastAddress;
				leftOverSize = freeList[headAddress].freeSpace - allocateSize;

				freeList[headAddress].freeSpace = allocateSize;
				freeList[headAddress].nextAddress = -2;

				// set up allocated list block for print use
				allocatedPc.length = allocateSize;
				allocatedPc.postion = headAddress;
				allocList.push_back(allocatedPc);

				pointerAllocated = headAddress;

				headAddress += allocateSize;

				if (headAddress < (int64_t)freeList.size())
				{
					freeList[headAddress].previousBlock = allocateSize;
					freeList[headAddress].nextAddress = -1;
					freeList[headAddress].freeSpace = leftOverSize;
				}

				if(iterateLastNode){
					//if we had to iterate through a list (implying there were more node )re confique list for change of last node
					freeList[(int64_t)freeList[headAddress].index - freeList[headAddress].previousBlock].lastAddress = -1;
					freeList[headAddress].lastAddress = lastNextAddr;
					freeList[freeList[headAddress].lastAddress].nextAddress = headAddress;
					headAddress = headReminder;

				}

				break;
			}
		}
		else
		{
		
			headAddress = freeList[headAddress].nextAddress;

			if (headAddress == -1)// if end of list reached return as we have ran out of memory or can not find a large enough block
			{
				headAddress = headReminder;
				cout << "Cannot allocate: " << allocateSize << "kibibytes" << endl;
				return 1;
			}
			iterateLastNode = true;

		}
	}

	cout << "Allocated: " << allocateSize << "kibibytes starting at: " << pointerAllocated << endl;
	return 0;
}

int nextFit::freeBlock(int64_t indexAddress)
{
	int64_t currentAddr = 0;

	int64_t listSize[2] = {(int64_t)freeList.size(), (int64_t)allocList.size()};

	//address to remember if colasing a block
	int64_t collasAddr = 0;
	int64_t collasNextAddr = 0;
	int64_t collasPrevAddr = 0;

	// non-existent and or bad pointer free check
	if (indexAddress < 0 || indexAddress > listSize[0])
	{

		cout << "Cannot free a block starting at: "<< indexAddress << endl;

		return 1;
	}

	if (freeList[indexAddress].nextAddress != -2)
	{

		cout << "Cannot free a block starting at: "<< indexAddress << endl;

		return 2;
	}

	// free the block if it existed

	for (size_t i = 0; i < allocList.size(); i++)
	{

		if (allocList[i].postion == indexAddress)
		{

			allocList.erase(allocList.begin() + i);
			break;
		}
	}

	if (headAddress >= (int64_t)freeList.size())
	{

		freeList[indexAddress].nextAddress = -1;
	}
	else
	{
		freeList[indexAddress].nextAddress = headAddress;
	}

	freeList[headAddress].lastAddress = indexAddress;

	headAddress = indexAddress;
	cout << "Freed block starting at: " << indexAddress << endl; 
	// collascing

	// collascing peice under current block
	if ((indexAddress + freeList[indexAddress].freeSpace) < listSize[0])
	{

		collasAddr = indexAddress + freeList[indexAddress].freeSpace;

		if (freeList[collasAddr].lastAddress >= 0)
		{
			// set up variables with next and prev nodes
			collasPrevAddr = freeList[collasAddr].lastAddress;
			collasNextAddr = freeList[collasAddr].nextAddress;

			// remove block from list and set other to look
			freeList[collasPrevAddr].nextAddress = collasNextAddr;
			freeList[collasNextAddr].lastAddress = collasPrevAddr;

			// add free space collasc block to new freed block
			cout << "Coalesce at: " << collasAddr << " adding " << freeList[collasAddr].freeSpace << endl;
			freeList[indexAddress].freeSpace += freeList[collasAddr].freeSpace;

			// remove data from collasc block
			freeList[collasAddr].freeSpace = 0;
			freeList[collasAddr].lastAddress = -1;
			freeList[collasAddr].nextAddress = -1;
			freeList[collasAddr].previousBlock = -1;
		}
	}
	// collascing peice above current block

	// check list to see if there is a block above newly freed block
	currentAddr = freeList[headAddress].nextAddress;

	while (currentAddr >= 0)
	{

		if (((int64_t)freeList[currentAddr].index + freeList[currentAddr].freeSpace) == indexAddress)
		{

			collasAddr = currentAddr;

			// set up variables with next and prev nodes
			collasPrevAddr = freeList[collasAddr].lastAddress;
			collasNextAddr = freeList[collasAddr].nextAddress;

			// remove block from list and set other to look
			freeList[collasPrevAddr].nextAddress = collasNextAddr;
			freeList[collasNextAddr].lastAddress = collasPrevAddr;

			// add free space collasc block to new freed block
			cout << "Coalesce at: " << indexAddress << " adding " << freeList[indexAddress].freeSpace << endl;
			freeList[collasAddr].freeSpace += freeList[indexAddress].freeSpace;

			// make new head
			freeList[collasAddr].lastAddress = -1;
			freeList[collasAddr].nextAddress = freeList[indexAddress].nextAddress;
			freeList[freeList[indexAddress].nextAddress].lastAddress = collasAddr;
			headAddress = collasAddr;

			// remove data from collasc block
			freeList[indexAddress].freeSpace = 0;
			freeList[indexAddress].lastAddress = -1;
			freeList[indexAddress].nextAddress = -1;
			freeList[indexAddress].previousBlock = -1;

			break;
		}

		currentAddr = freeList[currentAddr].nextAddress;
	}

	return 0;
}

void nextFit::printList()
{

	int64_t currentAddr = headAddress;
	int64_t iterator = 0;
	int64_t blockSize = 0;

	int64_t listSize[2] = {(int64_t)freeList.size(), (int64_t)allocList.size()};

	if (headAddress < listSize[0])
	{
		cout << "Free List" << endl;
		cout << " Index   Start  Length" << endl;

		while (currentAddr >= 0)
		{

			blockSize = freeList[currentAddr].freeSpace;

			cout << '[' << iterator << "]	" << currentAddr << "	" << blockSize << endl;

			iterator++;
			currentAddr = freeList[currentAddr].nextAddress;
		}
	}
	else
	{

		cout << "Free List is empty" << endl;
	}

	cout << endl;

	if (listSize[1] > 0)
	{
		sort(allocList.begin(),allocList.end(), sortByPos);

		cout << " Allocated List" << endl
			 << " Index   Start  Length" << endl;

		for (size_t i = 0; i < allocList.size(); i++)
		{

			cout << '[' << i << "]	" << allocList[i].postion << "	" << allocList[i].length << endl;
		}
	}
	else
	{

		cout << "Allocated List is empty" << endl;
	}
};



int handleArgs(int argcInput, char **argvInput, programArgHandler &argHandlerInput)
{

	int getOptReturn = 0;

	while ((getOptReturn = getopt(argcInput, argvInput, "hk:")) != -1)
	{

		switch (getOptReturn)
		{
		case 'h':

			argHandlerInput.helpComd = true;

			break;
		case 'k':

			argHandlerInput.kibiyts = strtoll(optarg, nullptr, 0);

			break;
		default:
			argHandlerInput.helpComd = true;

			return 1;
			break;
		}
	}

	return 0;
}

void parseInput(userInput &parseInputStruct)
{

	string textToParse;

	getline(cin, textToParse);

	parseInputStruct.userCommand = textToParse[0];

	if (parseInputStruct.userCommand == 'a' || parseInputStruct.userCommand == 'f')
	{

		textToParse = textToParse.substr(2, textToParse.size());

		parseInputStruct.userValue = strtoll(textToParse.c_str(), nullptr, 0);
	}
}

int memAllocatorProgram(bool isTerminal, int64_t intialSize)
{// main program routine for allocating 

	userInput userInputStuct;
	nextFit fitClass(intialSize);

	bool programLoopCondidtion = true;

	while (programLoopCondidtion)
	{

		if (isTerminal)
		{

			cout << "> ";
			parseInput(userInputStuct);
		}
		else
		{

			parseInput(userInputStuct);
		}

		switch (userInputStuct.userCommand)
		{
		case 'a':

			fitClass.allocate(userInputStuct.userValue);

			break;
		case 'f':

			if(fitClass.freeBlock(userInputStuct.userValue) > 0){

				return 1;

			}

			break;
		case 'p':

			fitClass.printList();

			break;
		case 'q':
			programLoopCondidtion = false;
			break;

		default:
			break;
		}
	}

	return 0;
};

int main(int argc, char *argv[])
{

	programArgHandler argHandler;

	if (handleArgs(argc, argv, argHandler))
	{

		cout << helpMessageString << endl;

		return 1;
	}

	if (argHandler.helpComd)
	{

		cout << "List of program Arguments" << endl
			 << helpMessageString << endl;

		return 0;
	}

	if (isatty(STDIN_FILENO))
	{

		if(memAllocatorProgram(true, argHandler.kibiyts) > 0){

			return 1;

		}
	}
	else
	{

		if (errno == ENOTTY)
		{
			if(memAllocatorProgram(false, argHandler.kibiyts) > 0){

				return 1;

			}
		}
		else
		{

			perror("problem with isatty(): ");
			return 1;
		}
	}

	return 0;
};
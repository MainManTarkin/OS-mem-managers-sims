#include <iostream>
#include <vector>

using namespace std;

class slabAllocator
{
private:
	vector<void *> memoryStack;
	vector<void *> pointerOffsets;

	u_int64_t totalMemorySize;

	int gulping();

public:
	slabAllocator();

	void *slabLoc();

	int freeSlab(void *slabPoint);

	size_t getStackSize();

	~slabAllocator();
};

slabAllocator::slabAllocator()
{

	totalMemorySize = 0;
};

int slabAllocator::gulping()
{

	void *gulpPointer = malloc(128 * 512);

	if (gulpPointer == nullptr)
	{

		perror("problem with malloc in gulping(): ");

		return 1;
	}
	pointerOffsets.push_back(gulpPointer);
	memoryStack.push_back(gulpPointer);

	for (size_t i = 1; i < 128; i++)
	{
		gulpPointer = (char *)gulpPointer + 512;

		memoryStack.push_back(gulpPointer);
	}

	totalMemorySize += 128 * 512;

	cout << "Gulping" << endl;

	return 0;
};

void *slabAllocator::slabLoc()
{

	void *slabPoint = nullptr;

	if (memoryStack.size() <= 0)
	{

		if (gulping() == 1)
		{

			return slabPoint;
		}
	}

	slabPoint = memoryStack.back();

	memoryStack.pop_back();

	return slabPoint;
};

int slabAllocator::freeSlab(void *slabPoint)
{
	char *pointerArt = nullptr;
	bool notFoundMatch = true;

	if (slabPoint == nullptr)
	{

		cout << "Correctly caught error: attempting to free nullptr" << endl;

		return 1;
	}

	if ((memoryStack.size() * 512) >= totalMemorySize)
	{

		cout << "Correctly caught error: free'd too many slabs" << endl;

		return 2;
	}

	for (size_t i = 0; i < pointerOffsets.size(); i++)
	{
		pointerArt = (char *)pointerOffsets[i];
		pointerArt = pointerArt + (128 * 512);

		if (pointerArt >= slabPoint && slabPoint >= pointerOffsets[i])
		{

			notFoundMatch = false;
			break;
		}
	}

	if (notFoundMatch)
	{

		cout << "Correctly caught error: attempting to free location not in any gulp" << endl;

		return 3;
	}

	if (memoryStack.size() != 0)
	{

		for (size_t i = memoryStack.size() - 1; i > 0; i--)
		{
			if (memoryStack[i] == slabPoint)
			{

				cout << "Correctly caught error: attempting double free" << endl;

				return 4;
			}
		}
	}

	memoryStack.push_back(slabPoint);

	return 0;
};

size_t slabAllocator::getStackSize()
{

	return memoryStack.size();
}

slabAllocator::~slabAllocator()
{
	// free correct amount of gulps
	cout << "Program Ending - Destructors should now run" << endl;

	for (size_t i = 0; i < pointerOffsets.size(); i++)
	{
		free(pointerOffsets[i]);

		cout << "Releasing Gulp" << endl;
	}
}

int main(int argc, char *argv[])
{
	void *pointStar = nullptr;

	void *pointStars[2] = {nullptr, nullptr};

	vector<void *> bufferStore;

	slabAllocator slabAlloc;
	// allocate 256 slabs
	for (size_t i = 128; i > 0; i--)
	{
		pointStar = slabAlloc.slabLoc();

		if (pointStar == nullptr)
		{

			return 1;
		}

		bufferStore.push_back(pointStar);

		cout << "Allocating Slab    " << i - 1 << endl;
	}

	for (size_t i = 128; i > 0; i--)
	{
		pointStar = slabAlloc.slabLoc();

		if (pointStar == nullptr)
		{

			return 1;
		}

		bufferStore.push_back(pointStar);

		cout << "Allocating Slab    " << i - 1 << endl;
	}

	if (slabAlloc.getStackSize() == 0)
	{

		cout << "Number of Available Slabs should be 0. Is: 0 (Correct)" << endl;
	}
	else
	{

		cout << "Number of Available Slabs should be 0. Is: " << slabAlloc.getStackSize() << " (Wrong)" << endl;

		return 1;
	}
	// free them now test

	for (size_t i = 0; i < bufferStore.size(); i++)
	{

		if (slabAlloc.freeSlab(bufferStore[i]) != 0)
		{

			return 1;
		}
	}

	if (slabAlloc.getStackSize() == 256)
	{

		cout << "Number of Available Slabs should be 256. Is: 256 (Correct)" << endl;
	}
	else
	{

		cout << "Number of Available Slabs should be 256. Is: " << slabAlloc.getStackSize() << " (Wrong)" << endl;
		return 1;
	}

	// Alloc / Free / Alloc Test
	pointStars[0] = slabAlloc.slabLoc();

	cout << "Allocating Slab  255" << endl;

	slabAlloc.freeSlab(pointStars[0]);

	pointStars[1] = slabAlloc.slabLoc();

	cout << "Allocating Slab  255" << endl;

	if (pointStars[0] == pointStars[1])
	{

		cout << "Alloc / Free / Alloc Test succeeded" << endl;
	}
	else
	{

		cout << "Alloc / Free / Alloc Test \" failed" << endl;

		return 1;
	}

	slabAlloc.freeSlab(pointStars[1]);

	//free nullptr test

	if (slabAlloc.freeSlab(nullptr) != 1)
	{

		cout << "Did Not Catch Attempt to Free NULL" << endl;

		return 1;
	}

	//free'd too many slabs test
	if (slabAlloc.freeSlab(pointStars[1]) != 2)
	{

		cout << "Did Not Catch Freeing of Too Many Buffers" << endl;

		return 1;
	}

	//double free test
	pointStars[0] = slabAlloc.slabLoc();

	cout << "Allocating Slab  255" << endl;

	pointStars[1] = slabAlloc.slabLoc();

	cout << "Allocating Slab  254" << endl;

	slabAlloc.freeSlab(pointStars[1]);

	if(slabAlloc.freeSlab(pointStars[1]) != 4){

		cout << "Did Not Catch Attempt to Double Free" << endl;

	}
	//bad addr test

	if(slabAlloc.freeSlab((void*)12345) != 3){

		cout << "Did Not Catch Attempt to Free BAD Address" << endl;

	}
	

	return 0;
}

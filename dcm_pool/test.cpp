// FastDcmPool.cpp : Defines the entry point for the console application.
//
#pragma once
#include <include/dcm_pool/dcm_pool.h>
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <list>

using namespace dcm_pool;

/*!
 * \class	Test
 *
 * \brief	Test class to check pool performance.
 *
 * \author	Ronen
 * \date	2/23/2018
 */
class Test
{
private:
	int _hp;

public:

	// init the object
	void Init()
	{
		_hp = std::rand() % 25 + 1;
	}

	// update object
	void Update()
	{
		if (std::rand() % 1000 <= 1)
			_hp--;
	}

	// check if object is dead
	inline bool is_dead() const { return _hp < 0; }
};

// list of objects to remove from test pool
std::list<ObjectId> _to_remove;

// count total update calls
int total_update_calls = 0;

// do all objects loop
void update_loop(Test& obj, ObjectId id)
{
	// update
	obj.Update();

	// increase total update calls
	total_update_calls++;

	// if object is dead, add to remove-list to remove it later
	if (obj.is_dead())
		_to_remove.push_back(id);
}


// for list remove_if
bool IsDead(Test& i) { return i.is_dead(); }



/*!
 * \fn	int main()
 *
 * \brief	Main entry-point for this application.
 *
 * \author	Ronen
 * \date	2/21/2018
 *
 * \return	Exit-code for the process - 0 for success, else an error code.
 */
int main()
{
	// get a seed for both tests
	std::srand((unsigned int)std::time(nullptr));
	int seed = std::rand();
	
	// some consts
	const int measure_frames_count = 15000;
	const int total_test_measure_times = 5;
	
	// test dcm_pool
	std::cout << endl << endl << "==========================" << endl;
	std::cout << "TEST DCM_POOL" << endl;
	std::cout << "==========================" << endl << endl;
	std::srand(seed);
	{
		// create pool
		DcmPool<Test> pool;

		// run for X frames and measure performance
		double total_iterations_time = 0.0;
		double total_remove_time = 0.0;
		double total_allocation_time = 0.0;
		int total_removed = 0;
		int total_added = 0;
		for (int i = 0; i < measure_frames_count * total_test_measure_times; ++i)
		{
			// check if time to alloc new object
			{
				clock_t begin = clock();
				DcmPool<Test>::Ptr new_obj = pool.Alloc();
				new_obj->Init();
				clock_t end = clock();
				total_allocation_time += double(end - begin) / CLOCKS_PER_SEC;
				total_added++;
			}

			// do per-frame update loop
			{
				clock_t begin = clock();
				pool.Iterate(update_loop);
				clock_t end = clock();
				total_iterations_time += double(end - begin) / CLOCKS_PER_SEC;
			}

			// do objects removing loop
			{
				clock_t begin = clock();
				for (auto iter = _to_remove.begin(); iter != _to_remove.end(); ++iter)
				{
					pool.Release(*iter);
				}
				clock_t end = clock();
				total_remove_time += double(end - begin) / CLOCKS_PER_SEC;
				total_removed += (int)_to_remove.size();
				_to_remove.clear();
			}

			// print info
			if (i % measure_frames_count == 0)
			{
				std::cout << "Frame: " << (i / measure_frames_count) << std::endl;
				std::cout << "Loops per frame: " << measure_frames_count << std::endl;
				std::cout << "Pool current size: " << pool.size() << std::endl;
				std::cout << "Total update calls: " << total_update_calls << std::endl;
				std::cout << "Iterations total time: " << total_iterations_time << std::endl;
				std::cout << "Allocations total time: " << total_allocation_time << std::endl;
				std::cout << "Remove objects total time: " << total_remove_time << std::endl;
				std::cout << "Objects removed this frame: " << total_removed << std::endl;
				std::cout << "Objects added this frame: " << total_added << std::endl;
				std::cout << "--------------------------" << std::endl;
				total_iterations_time = 0.0;
				total_update_calls = 0;
				total_remove_time = 0.0;
				total_removed = 0;
				total_added = 0;
			}
		}
	}


	_to_remove.clear();
	total_update_calls = 0;


	// test list
	std::cout << endl << endl << "==========================" << endl;
	std::cout << "TEST LIST" << endl;
	std::cout << "==========================" << endl << endl;
	std::srand(seed);
	{
		// create list
		std::list<Test> pool;

		// run for X frames and measure performance
		double total_iterations_time = 0.0;
		double total_remove_time = 0.0;
		double total_allocation_time = 0.0;
		int total_removed = 0;
		int total_added = 0;
		for (int i = 0; i < measure_frames_count * total_test_measure_times; ++i)
		{
			// check if time to alloc new object
			{
				clock_t begin = clock();
				Test new_obj;
				new_obj.Init();
				pool.push_back(new_obj);
				clock_t end = clock();
				total_allocation_time += double(end - begin) / CLOCKS_PER_SEC;
				total_added++;
			}

			// do per-frame update loop
			{
				clock_t begin = clock();
				int index = 0;
				for (auto iter = pool.begin(); iter != pool.end(); ++iter)
				{
					iter->Update();
					total_update_calls++;
					if (iter->is_dead())
						_to_remove.push_back(index);
					index++;
				}
				clock_t end = clock();
				total_iterations_time += double(end - begin) / CLOCKS_PER_SEC;
			}

			// do objects removing loop
			{
				clock_t begin = clock();
				pool.remove_if(IsDead);
				clock_t end = clock();
				total_remove_time += double(end - begin) / CLOCKS_PER_SEC;
				total_removed += (int)_to_remove.size();
				_to_remove.clear();
			}

			// print info
			if (i % measure_frames_count == 0)
			{
				std::cout << "Frame: " << (i / measure_frames_count) << std::endl;
				std::cout << "Loops per frame: " << measure_frames_count << std::endl;
				std::cout << "Pool current size: " << pool.size() << std::endl;
				std::cout << "Total update calls: " << total_update_calls << std::endl;
				std::cout << "Iterations total time: " << total_iterations_time << std::endl;
				std::cout << "Allocations total time: " << total_allocation_time << std::endl;
				std::cout << "Remove objects total time: " << total_remove_time << std::endl;
				std::cout << "Objects removed this frame: " << total_removed << std::endl;
				std::cout << "Objects added this frame: " << total_added << std::endl;
				std::cout << "--------------------------" << std::endl;
				total_iterations_time = 0.0;
				total_update_calls = 0;
				total_remove_time = 0.0;
				total_removed = 0;
				total_added = 0;
			}
		}
	}

	_to_remove.clear();
	total_update_calls = 0;

	// test vector
	std::cout << endl << endl << "==========================" << endl;
	std::cout << "TEST VECTOR" << endl;
	std::cout << "==========================" << endl << endl;
	std::srand(seed);
	{
		// create list
		std::vector<Test> pool;

		// run for X frames and measure performance
		double total_iterations_time = 0.0;
		double total_remove_time = 0.0;
		double total_allocation_time = 0.0;
		int total_removed = 0;
		int total_added = 0;
		for (int i = 0; i < measure_frames_count * total_test_measure_times; ++i)
		{
			// check if time to alloc new object
			{
				clock_t begin = clock();
				Test new_obj;
				new_obj.Init();
				pool.push_back(new_obj);
				clock_t end = clock();
				total_allocation_time += double(end - begin) / CLOCKS_PER_SEC;
				total_added++;
			}

			// do per-frame update loop
			{
				clock_t begin = clock();
				for (size_t i = 0; i < pool.size(); ++i)
				{
					Test& obj = pool[i];
					obj.Update();
					total_update_calls++;
					if (obj.is_dead())
						_to_remove.push_back(i);
				}
				clock_t end = clock();
				total_iterations_time += double(end - begin) / CLOCKS_PER_SEC;
			}

			// do objects removing loop
			{
				clock_t begin = clock();
				for (auto i = pool.size() - 1; i > 0; --i)
				{
					if (pool[i].is_dead())
					{
						pool.erase(pool.begin() + i);
					}
				}
				clock_t end = clock();
				total_remove_time += double(end - begin) / CLOCKS_PER_SEC;
				total_removed += (int)_to_remove.size();
				_to_remove.clear();
			}

			// print info
			if (i % measure_frames_count == 0)
			{
				std::cout << "Frame: " << (i / measure_frames_count) << std::endl;
				std::cout << "Loops per frame: " << measure_frames_count << std::endl;
				std::cout << "Pool current size: " << pool.size() << std::endl;
				std::cout << "Total update calls: " << total_update_calls << std::endl;
				std::cout << "Iterations total time: " << total_iterations_time << std::endl;
				std::cout << "Allocations total time: " << total_allocation_time << std::endl;
				std::cout << "Remove objects total time: " << total_remove_time << std::endl;
				std::cout << "Objects removed this frame: " << total_removed << std::endl;
				std::cout << "Objects added this frame: " << total_added << std::endl;
				std::cout << "--------------------------" << std::endl;
				total_iterations_time = 0.0;
				total_update_calls = 0;
				total_remove_time = 0.0;
				total_removed = 0;
				total_added = 0;
			}
		}
	}

	std::cin.get();
    return 0;
}


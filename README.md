# dcm_pool
Dynamic, Contiguous-Memory Objects Pool.

## What is it

A dynamic contiguous-memory Pool, or in short a dcm_pool, is a pool of objects that guarantee the following:

1. All objects are kept in memory in a contiguous memory block, and iterating them is equivalent to iterating a vector.
2. Accessing objects by pointer is O(1).
3. Allocating new objects is O(1), and does not cause an actual 'new()' call every time.
4. Releasing an object is O(1), and does not cause an actual 'delete()' call every time.
5. The pool can be dynamic, you don't have to set a hard size limit or allocate up-front.

## Why

This pool have very specific use cases, so best way to describe it is by example. 

Lets say you're building something with an [Entity-Component-System](https://en.wikipedia.org/wiki/Entity%E2%80%93component%E2%80%93system) design pattern, which is very common with video game engines. In ECS design pattern you have a basic 'entity' object, to which you attach different 'components' that implement different features and behaviors. 

Now you have different type of components attached to different entities, and they all require an 'Update()' call every frame. You have no way to tell how many components you'll need, and you probably want to pool your components so that repeated assigning and releasing won't cause too many new() and delete() calls. In addition, you want similar components to be in contiguous memory, to enjoy [memory access optimizations](https://bitbashing.io/memory-performance.html).

If you use a regular pool, you can easily reduce the new() and delete() calls, but your objects memory will be scattered and iterating them for the Update() loop would be less efficient. If you use a vector or other contiguous-memory container you'll enjoy a very fast iteration, but slow releasing / allocating.

The provided pool in this lib tries to provide something that will reduce new() / delete() calls, and also keep everything in contiguous memory. In addition, you don't have to sacrifice direct access (via pointer-like objects), although accessing via pointers would be slightly less efficient.

## Install

dcm_pool is a header-only lib, you don't need to compile any .lib or .dll files.

To use it, simply get the header files from [dcm_pool/include/](https://github.com/RonenNess/dcm_pool/tree/master/dcm_pool/include/) and make them available to your compiler.

## Usage

First lets take a look at a full, simple example:

```cpp
#include <dcm_pool/objects_pool.h>
using namespace dcm_pool;

// a dog class - what we'll have in pool
class Dog
{
public:

	// what dogs do every frame
	void Update()
	{
		cout << "Woof!" << endl;
	}

	// rub the dog's belly
	void RubBelly()
	{
		cout << "Your happiness increased by 5%." << endl;
	}
};

// main
void main()
{
	// create the pool of dogs
	ObjectsPool<Dog> pool;
	
	// create a dog and rub its belly
	auto new_dog = pool.Alloc();
	new_dog->RubBelly();
	
	// create another dog 
	auto new_dog2 = pool.Alloc();
	
	// release the first dog
	pool.Release(new_dog);
	
	// iterate over all dogs in pool and update them using lambda
	pool.Iterate([](Dog& dog, ObjectId id) { return dog.Update(); });
}
```

Now lets dive into the details:

### Limitations & Tips

1. The objects you use in pool must have a default constructor.
2. Allocating new objects will not invoke a constructor call. You need to create and call an Init() function manually.
3. Releasing objects will not invoke a destructor call. You need to create and call a Dtor() function manually.
4. As an extension of the constructor / destructor limitation, if you do implement them they shouldn't do any heavy lifting as they may be called internally.
5. Implementing a Move Assignment Operator will increase performance significantly.
6. To maximize the memory-based optimization, don't use the pool to store pointers or references. 

As you can see the limitations above apply to most basic pooling solutions. Nothing too special.

### Creating A Pool

Creating a new pool is easy:

```cpp
ObjectsPool<MyObjectType> pool;
```

Note that its best to use the object type itself, and not a reference or a pointer. If you use pointers you'll lose some of the memory-access performance and there's no reason for that.

The objects pool constructor receive several optional params to help you fine-tune its behavior:

```cpp
ObjectsPool(size_t max_size = 0, size_t pre_alloc = 0, size_t shrink_threshold = 1024, DefragModes defrag_mode = DEFRAG_DEFERRED);
```

- **max_size**: If provided, will limit the pool size (throw exception if exceeding limit).
- **pre_alloc**: If provided, will allocate this pool size upfront (translate to vector's reserve).
- **shrink_threshold**: While the pool grows dynamically, we only shrink the pool's memory chunk when having this amount of free objects in pool.
- **defrag_mode**: When to handle defragging - immediately on release, when trying to iterate objects, or manually.


### Allocating & Releasing

To allocate a new object from pool you need to use ```Alloc```:

```cpp
auto newobj = pool.Alloc();
```

Note that this will not invoke the object's constructor. If you want an initialize function you need to define one and call it manually.

The returned value of ```Alloc``` is a pointer-like object that provide a direct access to the object in pool. Even after defragging, the pointer will not lose its reference (however, don't try to grab its actual address manually as it might change later).

Note that accessing the pointer directly is not as efficient as using a regular pointer and may require a hash-table lookup if the pool was defragged.

When you want to release the object, you use ```Release```:

```cpp
pool.Release(newobj);
```

### Iterating Pool

The main way to iterate a pool is via the ```Iterate``` function. With a lambda, it looks like this:

```cpp
pool.Iterate([](MyObjectType& obj, ObjectId id) { /* do something with object */ });
```

Or you can use a proper function:

```cpp
// iteration callback
void update_loop(MyObjectType& obj, ObjectId id)
{
	// your update code here
}

// using the iteration callback:
pool.Iterate(update_loop);
```

Or if you need a finer control, you can use ```IterateEx```:

```cpp
// iteration callback
IterationReturnCode update_loop(MyObjectType& obj, ObjectId id, ObjectsPool<MyObjectType>& pool)
{
	// your update code here
	
	// continue to next object
	return IterationReturnCode::ITER_CONTINUE;
}

// using the iteration callback:
pool.IterateEx(update_loop);
```

### Cleanup

The pool with shrink its used memory automatically when there are too many unused objects in pool. However, if you want to reduce the pool's size to minimum immediately, you can call:

```cpp
pool.ClearUnusedMemory();
```

Note that if the pool is not defragged (eg have holes in it) it will raise exception.

### Defragging

As mentioned before, the pool might have "holes" in its contiguous memory due to objects being released from the middle. To solve this, the dcm_pool do self-defragging.

The defragging process worst case takes O(N), where N is number of holes in the pool and not number of objects.

dcm_pool Support 3 defragging modes:

#### DEFRAG_IMMEDIATE

Will close the hole immediately as its created. With this mode the performance is deterministic; The moment you release an object that creates a hole, it will be closed by moving the last object in its place.

#### DEFRAG_DEFERRED

In this mode the pool will accumulate holes and only close them when trying to iterate the pool. The huge advantage here is that if you do something like Release -> Alloc -> Release -> Alloc, in the two times you alloc it will just return the unused objects in the middle, and no will defragging will happen.

The disadvantage is that the performance of the Iteration() call becomes non-deterministic and may change depending on how many holes you have in your contiguous memory.

#### DEFRAG_MANUAL

In this mode the pool will never defrag on its own. If you iterate a pool with holes it will just skip the unused objects, and you'll need to call ```pool.Defrag()``` manually when you think its right.


## How does it work

1. The pool uses a vector internally to store the objects in memory.
2. The vector grows and shrink as the pool changes its size.
3. When you release an object, it creates a 'hole' in the contiguous vector memory. 
	3. a. That hole is closed during the defragging process. 
	3. b. We can accumulate a list of holes if defragging mode is not immediate.
	3. c. Defragging takes O(N), where N is number of holes (not number of pool). We close holes by taking objects from the end of the vector.
4. To iterate the vector you call Iterate(), which takes a function pointer to run on every valid object in pool.
5. Since defragging shuffles memory, you can't access objects by index. To solve this, we use a special pointer class to allow access to specific objects:
	5. a. Every object in pool is asigned with a unique id.
	5. b. The pool keeps an internal hash table to convert id to actual index (hidden from the user).
	5. c. When the pointer tries to fetch the object it points on, if the pool was defragged since last access it use the hash table to find the new objects index.
6. To store the list of holes in the vector we reuse the free objects, so no additional memory is wasted.

## Performance

To test the dcm_pool I created a simple object type that has an hp counter and an Update() function that randomly decreases it. When reaching 0, the object is dead and is removed.

I simulate few 'Update' loops, where in every loop I create new 15000 objects, add them to pool, and iterate all the existing objects in pool to Update them. Then I measure how long it took to update everything, adding the new objects, and deleting the dead ones.

I compared the 'dcm_pool' performance to a simple list and a vector naive implementation.

The test is not very 'fair' because the list and vector are used in a very naive way, but it still gives a general idea about how efficient 'dcm_pool' is.

Note: Although possible, I didn't allocate memory upfront for the pool nor the vector.

The results are below:

```
==========================
TEST DCM_POOL
==========================

Frame: 0
Loops per frame: 15000
Pool current size: 1
Total update calls: 0
Iterations total time: 0
Allocations total time: 0
Remove objects total time: 0
Objects removed this frame: 0
Objects added this frame: 1
--------------------------
Frame: 1
Loops per frame: 15000
Pool current size: 6950
Total update calls: 72430695
Iterations total time: 23.246
Allocations total time: 0.155
Remove objects total time: 0.159
Objects removed this frame: 8051
Objects added this frame: 15000
--------------------------
Frame: 2
Loops per frame: 15000
Pool current size: 6970
Total update calls: 104489064
Iterations total time: 33.579
Allocations total time: 0.286
Remove objects total time: 0.277
Objects removed this frame: 14980
Objects added this frame: 15000
--------------------------
Frame: 3
Loops per frame: 15000
Pool current size: 6950
Total update calls: 104463885
Iterations total time: 33.439
Allocations total time: 0.406
Remove objects total time: 0.28
Objects removed this frame: 15020
Objects added this frame: 15000
--------------------------
Frame: 4
Loops per frame: 15000
Pool current size: 6831
Total update calls: 104115381
Iterations total time: 33.994
Allocations total time: 0.531
Remove objects total time: 0.262
Objects removed this frame: 15119
Objects added this frame: 15000
--------------------------


==========================
TEST LIST
==========================

Frame: 0
Loops per frame: 15000
Pool current size: 1
Total update calls: 1
Iterations total time: 0
Allocations total time: 0
Remove objects total time: 0
Objects removed this frame: 0
Objects added this frame: 1
--------------------------
Frame: 1
Loops per frame: 15000
Pool current size: 6976
Total update calls: 72635680
Iterations total time: 51.905
Allocations total time: 0.023
Remove objects total time: 36.75
Objects removed this frame: 8025
Objects added this frame: 15000
--------------------------
Frame: 2
Loops per frame: 15000
Pool current size: 6952
Total update calls: 104812066
Iterations total time: 75.659
Allocations total time: 0.07
Remove objects total time: 53.76
Objects removed this frame: 15024
Objects added this frame: 15000
--------------------------
Frame: 3
Loops per frame: 15000
Pool current size: 6935
Total update calls: 104436172
Iterations total time: 75.233
Allocations total time: 0.118
Remove objects total time: 53.833
Objects removed this frame: 15017
Objects added this frame: 15000
--------------------------
Frame: 4
Loops per frame: 15000
Pool current size: 6949
Total update calls: 104269843
Iterations total time: 73.96
Allocations total time: 0.157
Remove objects total time: 52.76
Objects removed this frame: 14986
Objects added this frame: 15000
--------------------------


==========================
TEST VECTOR
==========================

Frame: 0
Loops per frame: 15000
Pool current size: 1
Total update calls: 1
Iterations total time: 0
Allocations total time: 0
Remove objects total time: 0
Objects removed this frame: 0
Objects added this frame: 1
--------------------------
Frame: 1
Loops per frame: 15000
Pool current size: 6872
Total update calls: 71811310
Iterations total time: 25.1
Allocations total time: 0.019
Remove objects total time: 12.152
Objects removed this frame: 15735
Objects added this frame: 15000
--------------------------
Frame: 2
Loops per frame: 15000
Pool current size: 6957
Total update calls: 103485217
Iterations total time: 36.328
Allocations total time: 0.031
Remove objects total time: 17.489
Objects removed this frame: 29915
Objects added this frame: 15000
--------------------------
Frame: 3
Loops per frame: 15000
Pool current size: 6972
Total update calls: 104276134
Iterations total time: 36.259
Allocations total time: 0.052
Remove objects total time: 17.383
Objects removed this frame: 29985
Objects added this frame: 15000
--------------------------
Frame: 4
Loops per frame: 15000
Pool current size: 6918
Total update calls: 104338469
Iterations total time: 36.525
Allocations total time: 0.065
Remove objects total time: 17.633
Objects removed this frame: 30054
Objects added this frame: 15000
--------------------------

```

#### Conclusion

At the time of peak (eg when there were the most objects in pools), we measured the following times (all in seconds):

```
                Dcm_pool        List        Vector    
Iterating       33.439          75.233      36.259
Allocating      0.406           0.118       0.052
Releasing       0.28            53.833      17.383
Update Calls    104463885       104436172   104276134
```


Note: the Vector was iterated using an iterator. However, if we iterate the vector using a direct index its much faster and closer to the pool's performance.

**If performance is pretty close to vector (despite in releasing) why not using a vector?**

From the benchmark above you may come to the conclusion that a vector may be 'good enough', provided you don't have lots of releasing to do. However, keep in mind that you can't safely hold a pointer to an item inside a vector, since pushing / poping may change the underling addresses. The dcm_pool however, gives you vector-like performance but with faster releasing AND safe-to-use pointers to objects inside the pool.

## License

dcm_pool is distributed under the MIT license and can be used for any purpose.

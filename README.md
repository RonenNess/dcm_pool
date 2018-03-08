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

All the examples below assume your compiler knows the dcm_pool headers folder and have the following statements:

```cpp
#include <dcm_pool/objects_pool.h>
using namespace dcm_pool;
```

### Creating A Pool

Creating a new pool is easy:

```
ObjectsPool<MyObjectType> pool;
```

Note that its best to use the object type itself, and not a reference or a pointer. If you use pointers you'll lose some of the memory-access performance and there's no reason for that.

The objects pool constructor receive several optional params to help you fine-tune its behavior:

```
ObjectsPool(size_t max_size = 0, size_t pre_alloc = 0, size_t shrink_threshold = 1024, DefragModes defrag_mode = DEFRAG_DEFERRED);
```

- **max_size**: If provided, will limit the pool size (throw exception if exceeding limit).

## How does it work

TBD

## Performance

To test the dcm_pool I created a simple object type that has an hp counter and an Update() function that randomly decreases it. When reaching 0, the object is dead and is removed.

I simulate few 'Update' loops, where in every loop I create new 15000 objects, add them to pool, and iterate all the existing objects in pool to Update them. Then I measure how long it took to update everything, adding the new objects, and deleting the dead ones.

I compared the 'dcm_pool' performance to a simple list and a vector naive implementation.

The test is not very 'fair' because the list and vector are used in a very naive way, but it still gives a general idea about how efficient 'dcm_pool' is.

Note: Although possible, I didn't allocate memory upfront for the pool nor the vector.

The results are below:

```

```

## License

dcm_pool is distributed under the MIT license and can be used for any purpose.

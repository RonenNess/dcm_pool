# dcm_pool
Dynamic, Contiguous-Memory Objects Pool.

## What is it

A dynamic contiguous-memory Pool, or in short a dcm_pool, is a pool of object that provides the following promises:

1. All objects are kept in memory in a contiguous memory block, and iterating them is equivalent to iterating a vector.
2. Accessing objects by pointer is O(1).
3. Allocating new objects is O(1).
4. Releasing an object is O(1), and does not cause an actual 'delete()' call.
5. The pool is dynamic, you don't have to set a constant size up-front.

## Why

This pool have very specific use cases, so best way to describe it is by example. 

Lets say you're building something with an [Entity-Component-System](https://en.wikipedia.org/wiki/Entity%E2%80%93component%E2%80%93system) design pattern, which is very common with video game engines. In ECS design pattern you have a basic 'entity' object, to which you attach different 'components' that implement different features and behaviors. 

Now you have different type of components attached to different entities, and they all require an 'Update()' call every frame. You have no way to tell how many components you'll need, and you probably want to pool your components so that repeated assigning and releasing won't cause too many new() and delete() calls. In addition, you want similar components to be in contiguous memory, to enjoy [memory access optimizations](https://bitbashing.io/memory-performance.html).

If you use a regular pool, you can easily reduce the new() and delete() calls, but your objects memory will be scattered and iterating them for the Update() loop would be less efficient. If you use a vector or other contiguous-memory container you'll enjoy a very fast iteration, but slow releasing / allocating.

The provided pool in this lib tries to provide something that both reduce new() / delete() calls, and also keep everything in contiguous memory so iterating your objects will be faster.

## Install

dcm_pool is a header-only lib, you don't need any .lib or .dll files.

To use it, fetch all the dcm_pool header files from [dcm_pool/include/](https://github.com/RonenNess/dcm_pool/tree/master/dcm_pool/include/) and make them available to your compiler.

## Usage

Assuming your compiler knows the dcm_pool header folder, lets first include and use the dcm_pool namespace:

```cpp
#include <dcm_pool/objects_pool.h>

using namespace dcm_pool;
```

## Performance

```

```

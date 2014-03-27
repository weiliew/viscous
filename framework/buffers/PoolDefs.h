/*
 * PoolDefs.h
 *
 *  Created on: 21 Mar 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef POOLDEFS_H_
#define POOLDEFS_H_

// frequently used types for buffers and factories
#include <boost/lockfree/stack.hpp>
#include <boost/lockfree/queue.hpp>
#include "buffers/FixedBuffer.h"
#include "buffers/BufferPool.h"
#include "buffers/LockedQueue.h"
#include "buffers/LockFreeQueue.h"

namespace vf_common
{

template<typename QueueT>
struct PoolDefs
{
    typedef typename QueueT::value_type     BufferType;
    typedef typename QueueT::value_ptr_type BufferPtrType;
    typedef QueueT                          QueueType;
    typedef BufferPool<QueueType>           PoolType;
    typedef PooledBufferFactory<PoolType>   PooledFactoryType;
};

typedef PoolDefs<LockedQueue<FixedBuffer<64>   > >   LockingFixedBuffer64;
typedef PoolDefs<LockedQueue<FixedBuffer<256>  > >   LockingFixedBuffer256;
typedef PoolDefs<LockedQueue<FixedBuffer<1024> > >   LockingFixedBuffer1k;
typedef PoolDefs<LockedQueue<FixedBuffer<2048> > >   LockingFixedBuffer2k;

typedef PoolDefs<LockFreeQueue<FixedBuffer<64> > >   LockFreeFixedBuffer64;
typedef PoolDefs<LockFreeQueue<FixedBuffer<64> > >   LockFreeFixedBuffer256;
typedef PoolDefs<LockFreeQueue<FixedBuffer<64> > >   LockFreeFixedBuffer1k;
typedef PoolDefs<LockFreeQueue<FixedBuffer<64> > >   LockFreeFixedBuffer2k;

// TODO - variable length Buffers -

}  // namespace vf_common


#endif /* POOLDEFS_H_ */

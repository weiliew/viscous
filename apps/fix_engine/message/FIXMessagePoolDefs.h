/*
 * FIXMessagePool.h
 *
 *  Created on: 26 Apr 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef FIXMESSAGEPOOLDEFS_H_
#define FIXMESSAGEPOOLDEFS_H_

#include "buffers/PoolDefs.h"
#include "FIXMessage.h"
#include "FIXMessageFactory.h"

namespace vf_fix
{

using vf_common::LockedQueue;
using vf_common::LockFreeQueue;
using vf_common::FixedBuffer;
using vf_common::BufferPool;

template<typename QueueT>
struct FIXMessagePoolDefs
{
    typedef typename QueueT::value_type             BufferType;
    typedef typename QueueT::value_ptr_type         BufferPtrType;
    typedef QueueT                                  QueueType;
    typedef BufferPool<QueueType>                   PoolType;
    typedef FIXMessageFactory<PoolType>             PooledFactoryType;
};

typedef FIXMessagePoolDefs<LockedQueue  <FIXMsg<FixedBuffer<2048>>>> LockingFIXMsg2k;
typedef FIXMessagePoolDefs<LockFreeQueue<FIXMsg<FixedBuffer<2048>>>> LockFreeFIXMsg2k;

}

#endif /* FIXMESSAGEPOOLDEFS_H_ */

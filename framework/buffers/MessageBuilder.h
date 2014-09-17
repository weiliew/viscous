/*
 * MessageBuilder.h
 *
 *  Created on: 10 May 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef MESSAGEBUILDER_H_
#define MESSAGEBUILDER_H_

/* MessageBuilder is a non thread safe class that allows single threaded callback receiver to construct/reconstruct
 * single messages to be passed onwards to the message handler or parser. This is particularly useful for
 * TCP/stream based io receivers.
 * This class will not make copies of the buffer supplied, but may make use of the factory to create additional
 * messages if necessary - i.e. when there's > 1 message in a single buffer passed in.
 * Requirements :
 * The incoming message type must match outgoing message type for efficiency
 */

namespace vf_common
{

template<typename SignalType, typename FactoryType, typename InlineIO>
class MessageBuilder
{
public:
    typedef typename SignalType::element_type   MsgType;

    MessageBuilder(SignalType& outgoingSignal, FactoryType& factory)
    : _factory(factory)
    , _msgSignal(outgoingSignal)
    {
        clear<MsgType>();
    }

    ~MessageBuilder()
    {
    }

    void onData(MsgType& msg)
    {
        processData<MsgType>(msg);
    }

    // if the message type is a shared pointer
    template<typename T>
    typename std::enable_if<!std::is_pointer<T>::value, void>::type
    processData(MsgType& msg)
    {
        if(_msgCache.get())
        {
            _msgCache->appendBuffer(msg->buffer(), msg->size());
        }
        else
        {
            _msgCache = msg;
        }

        size_t processed = _msgCache->getCompleteMsg();
        while(processed > 0)
        {
            if(_msgCache->size() > processed)
            {
                auto newCachedMsg = _factory.create();
                newCachedMsg->setBuffer(_msgCache->buffer()+processed, _msgCache->size()-processed);
                _msgCache->setSize(processed);

                processBuffer<InlineIO>(_msgCache);
                _msgCache = newCachedMsg;
                processed = _msgCache->getCompleteMsg();
            }
            else if(processed < 0)
            {
                // problem with the message, discard whole message
                _msgCache->clear();
                break;
            }
            else
            {
                processBuffer<InlineIO>(_msgCache);
                _msgCache = nullptr;
                break;
            }
        }
    }

    // if the message type is a raw pointer
    template<typename T>
    typename std::enable_if<std::is_pointer<T>::value, void>::type
    processData(MsgType& msg)
    {
        if(_msgCache)
        {
            _msgCache->appendBuffer(msg->buffer(), msg->size());
        }
        else
        {
            _msgCache = msg;
        }

        size_t processed = _msgCache->getCompleteMsg();
        while(processed > 0)
        {
            if(_msgCache->size() > processed)
            {
                auto newCachedMsg = _factory.create();
                newCachedMsg->setBuffer(_msgCache->buffer()+processed, _msgCache->size()-processed);
                _msgCache->setSize(processed);

                processBuffer<InlineIO>(_msgCache);
                _msgCache = newCachedMsg;
                processed = _msgCache->getCompleteMsg();
            }
            else if(processed < 0)
            {
                // problem with the message, discard whole message
                _msgCache->clear();
                break;
            }
            else
            {
                processBuffer<InlineIO>(_msgCache);
                _msgCache = NULL;
                break;
            }
        }
    }


    SignalType& getSignal()
    {
        return _msgSignal;
    }


    // if the message type is a shared pointer
    template<typename T>
    typename std::enable_if<!std::is_pointer<T>::value, void>::type
    clear()
    {
        _msgCache = nullptr;
    }

    // if the message type is a raw pointer
    template<typename T>
    typename std::enable_if<std::is_pointer<T>::value, void>::type
    clear()
    {
        _msgCache = NULL;
    }

private:
    template<typename T> // if T(InlineIO) is true
    typename std::enable_if<T::value, void>::type
    processBuffer(MsgType& buffer)
    {
        _msgSignal.dispatch(buffer);
    }

    template<typename T> // if T(InlineIO) is false
    typename std::enable_if<!T::value, void>::type
    processBuffer(MsgType& buffer)
    {
        _msgSignal.post(buffer);
    }

    FactoryType&    _factory;
    SignalType&     _msgSignal;
    MsgType         _msgCache;
};

}  // namespace vf_common


#endif /* MESSAGEBUILDER_H_ */

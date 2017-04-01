
// ============================================================ //
//
//   d88888D db   d8b   db  .d8b.  db    db
//   YP  d8' 88   I8I   88 d8' `8b `8b  d8'
//      d8'  88   I8I   88 88ooo88  `8bd8'
//     d8'   Y8   I8I   88 88~~~88    88
//    d8' db `8b d8'8b d8' 88   88    88
//   d88888P  `8b8' `8d8'  YP   YP    YP
//
//   open-source, cross-platform, crypto-messenger
//
//   Copyright (C) 2016 Marc Weiler
//
//   This library is free software; you can redistribute it and/or
//   modify it under the terms of the GNU Lesser General Public
//   License as published by the Free Software Foundation; either
//   version 2.1 of the License, or (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//   Lesser General Public License for more details.
//
// ============================================================ //

#ifndef ZWAY_TYPES_H_
#define ZWAY_TYPES_H_

#include <memory>
#include <deque>
#include <list>
#include <map>

// ============================================================ //

namespace Zway {

namespace UBJ {

class Value;

class Object;

class Array;

class Store;

class StoreHandler;

typedef std::shared_ptr<Store> STORE;

typedef std::shared_ptr<StoreHandler> STORE_HANDLER;

}

class Buffer;

class Packet;

class Engine;

class Client;

class StreamReceiver;

class StreamSender;

class BufferReceiver;

class BufferSender;

class UbjReceiver;

class UbjSender;

class ResourceReceiver;

class ResourceSender;

class Event;

class EventHandler;

class Request;

class RequestEvent;

class CreateAccountRequest;

class LoginRequest;

class LogoutRequest;

class ConfigRequest;

class AddContactRequest;

class CreateAddCodeRequest;

class FindContactRequest;

class AcceptContactRequest;

class RejectContactRequest;

class ContactStatusRequest;

class DispatchRequest;

class PushRequest;

class Message;

class Resource;

class Store;

typedef std::shared_ptr<Buffer> BUFFER;

typedef std::shared_ptr<Packet> PACKET;

typedef std::shared_ptr<Client> CLIENT;

typedef std::shared_ptr<StreamReceiver> STREAM_RECEIVER;

typedef std::map<uint32_t, STREAM_RECEIVER> STREAM_RECEIVER_MAP;

typedef std::function<void (STREAM_RECEIVER)> STREAM_RECEIVER_CALLBACK;

typedef std::shared_ptr<StreamSender> STREAM_SENDER;

typedef std::map<uint32_t, STREAM_SENDER> STREAM_SENDER_MAP;

typedef std::function<void (STREAM_SENDER)> STREAM_SENDER_CALLBACK;

typedef std::shared_ptr<BufferReceiver> BUFFER_RECEIVER;

typedef std::function<void (BUFFER_RECEIVER, BUFFER, uint32_t)> BUFFER_RECEIVER_CALLBACK;

typedef std::shared_ptr<BufferSender> BUFFER_SENDER;

typedef std::shared_ptr<UbjReceiver> UBJ_RECEIVER;

typedef std::function<void (UBJ_RECEIVER, UBJ::Value&)> UBJ_RECEIVER_CALLBACK;

typedef std::shared_ptr<UbjSender> UBJ_SENDER;

typedef std::shared_ptr<ResourceReceiver> RESOURCE_RECEIVER;

typedef std::shared_ptr<ResourceSender> RESOURCE_SENDER;

typedef std::shared_ptr<Event> EVENT;

typedef std::function<void (EVENT)> EVENT_CALLBACK;

typedef std::shared_ptr<EventHandler> EVENT_HANDLER;

typedef std::function<void (CLIENT,EVENT)> EVENT_HANDLER_CALLBACK;

typedef std::shared_ptr<Request> REQUEST;

typedef std::map<uint32_t, REQUEST> REQUEST_MAP;

typedef std::shared_ptr<RequestEvent> REQUEST_EVENT;

typedef std::function<void (REQUEST_EVENT, REQUEST)> REQUEST_CALLBACK;

typedef std::shared_ptr<CreateAccountRequest> CREATE_ACCOUNT_REQUEST;

typedef std::shared_ptr<LoginRequest> LOGIN_REQUEST;

typedef std::shared_ptr<LogoutRequest> LOGOUT_REQUEST;

typedef std::shared_ptr<ConfigRequest> CONFIG_REQUEST;

typedef std::shared_ptr<AddContactRequest> ADD_CONTACT_REQUEST;

typedef std::shared_ptr<CreateAddCodeRequest> CREATE_ADD_CODE_REQUEST;

typedef std::shared_ptr<FindContactRequest> FIND_CONTACT_REQUEST;

typedef std::shared_ptr<AcceptContactRequest> ACCEPT_CONTACT_REQUEST;

typedef std::shared_ptr<RejectContactRequest> REJECT_CONTACT_REQUEST;

typedef std::shared_ptr<ContactStatusRequest> CONTACT_STATUS_REQUEST;

typedef std::shared_ptr<DispatchRequest> DISPATCH_REQUEST;

typedef std::shared_ptr<PushRequest> PUSH_REQUEST;

typedef std::shared_ptr<Message> MESSAGE;

typedef std::deque<MESSAGE> MESSAGE_LIST;

typedef std::shared_ptr<Resource> RESOURCE;

typedef std::deque<RESOURCE> RESOURCE_LIST;

typedef std::map<uint32_t, RESOURCE> RESOURCE_MAP;

typedef std::shared_ptr<Store> STORE;

}

// ============================================================ //

#endif

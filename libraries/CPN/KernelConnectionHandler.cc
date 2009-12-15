//=============================================================================
//	Computational Process Networks class library
//	Copyright (C) 1997-2006  Gregory E. Allen and The University of Texas
//
//	This library is free software; you can redistribute it and/or modify it
//	under the terms of the GNU Library General Public License as published
//	by the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This library is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//	Library General Public License for more details.
//
//	The GNU Public License is available in the file LICENSE, or you
//	can write to the Free Software Foundation, Inc., 59 Temple Place -
//	Suite 330, Boston, MA 02111-1307, USA, or you can find it on the
//	World Wide Web at http://www.fsf.org.
//=============================================================================
/** \file
 * \author John Bridgman
 */

#include "KernelConnectionHandler.h"
#include "PacketDecoder.h"
#include "PacketEncoder.h"
#include "Database.h"

#include "SockHandler.h"
#include "Assert.h"
#include "ErrnoException.h"

#if 0
#define FUNC_TRACE(logger) SCOPE_TRACE(logger)
#else
#define FUNC_TRACE(logger)
#endif
namespace CPN {

    class KernelConnectionHandler::Connection 
        : public SockHandler, public Future<int>,
          public PacketDecoder, public PacketEncoder,
          public std::tr1::enable_shared_from_this<Connection>
    {
    public:
        enum Mode_t {
            WAITING,
            UNKNOWN,
            ID_READER,
            ID_WRITER,
            DEAD
        };

        Connection(KernelConnectionHandler &kch_, Sync::ReentrantLock &lock_, Mode_t mode_)
            : kch(kch_), lock(lock_), mode(mode_), readerkey(0), writerkey(0)
        {
            Writeable(false);
            kch.logger.Trace("New Connection(%d)", mode);
        }

        ~Connection() {
            kch.logger.Trace("Connection destroyed r:%lu w:%lu s:%u %s",
                    readerkey, writerkey, mode, Closed() ? "closed" : "open");
        }

        bool Dead() { return mode == DEAD; }
        // Future<int>
        virtual bool Done() {
            Sync::AutoReentrantLock arlock(lock);
            return mode == WAITING || mode == DEAD;
        }

        virtual void Cancel() {
            Sync::AutoReentrantLock arlock(lock);
            mode = DEAD;
            Close();
        }

        virtual int Get() {
            Sync::AutoReentrantLock arlock(lock);
            FUNC_TRACE(kch.logger);
            int ret = FD();
            FileHandler::Reset();
            mode = DEAD;
            return ret;
        }

        // FileHandler
        virtual void OnRead() {
            Sync::AutoReentrantLock arlock(lock);
            if (!Good()) { return; }
            ASSERT(!Done());
            FUNC_TRACE(kch.logger);
            try {
                bool loop = true;
                while (loop && Readable()) {
                    unsigned numtoread = 0;
                    void *ptr = PacketDecoder::GetDecoderBytes(numtoread);
                    unsigned numread = Recv(ptr, numtoread, false);
                    if (numread == 0) {
                        if (!Good()) {
                            // Eof
                            mode = WAITING;
                            Close();
                        }
                        loop = false;
                    } else {
                        PacketDecoder::ReleaseDecoderBytes(numread);
                    }
                }
            } catch (const ErrnoException &e) {
                mode = DEAD;
                Close();
            }
        }

        virtual void OnWrite() {}
        virtual void OnError() {
            Sync::AutoReentrantLock arlock(lock);
            mode = DEAD;
            Close();
        }
        virtual void OnHup() {
            Sync::AutoReentrantLock arlock(lock);
            mode = DEAD;
            Close();
        }
        virtual void OnInval() {
            Sync::AutoReentrantLock arlock(lock);
            mode = DEAD;
            Close();
        }
        virtual bool Readable() const {
            Sync::AutoReentrantLock arlock(lock);
            return mode == UNKNOWN;
        }

        // PacketHandler
        virtual void EnqueuePacket(const Packet &packet) { ASSERT(false); }
        virtual void DequeuePacket(const Packet &packet) { ASSERT(false); }
        virtual void ReadBlockPacket(const Packet &packet) { ASSERT(false); }
        virtual void WriteBlockPacket(const Packet &packet) { ASSERT(false); }
        virtual void EndOfWritePacket(const Packet &packet) { ASSERT(false); }
        virtual void EndOfReadPacket(const Packet &packet) { ASSERT(false); }
        virtual void IDReaderPacket(const Packet &packet) {
            Sync::AutoReentrantLock arlock(lock);
            FUNC_TRACE(kch.logger);
            readerkey = packet.SourceKey();
            writerkey = packet.DestinationKey();
            mode = WAITING;
            kch.Transfer(writerkey, shared_from_this());
        }
        virtual void IDWriterPacket(const Packet &packet) {
            Sync::AutoReentrantLock arlock(lock);
            FUNC_TRACE(kch.logger);
            readerkey = packet.DestinationKey();
            writerkey = packet.SourceKey();
            mode = WAITING;
            kch.Transfer(readerkey, shared_from_this());
        }
        // PacketEncoder
        virtual void WriteBytes(const iovec *iov, unsigned iovcnt) {
            Sync::AutoReentrantLock arlock(lock);
            int numwritten = Writev(iov, iovcnt);
            ASSERT(numwritten > 0);
        }

        Key_t ReaderKey() const { return readerkey; }
        void ReaderKey(Key_t k) { readerkey = k; }
        Key_t WriterKey() const { return writerkey; }
        void WriterKey(Key_t k) { writerkey = k; }
        Mode_t Mode() const { return mode; }
        void Mode(Mode_t m) { mode = m; }

        void Transfer(shared_ptr<Connection> conn) {
            Sync::AutoReentrantLock arlock(lock);
            FUNC_TRACE(kch.logger);
            ASSERT(Closed());
            mode = conn->Mode();
            ASSERT(readerkey == conn->ReaderKey());
            ASSERT(writerkey == conn->WriterKey());
            FD(conn->Get());
        }

        void SendIDReader() {
            Sync::AutoReentrantLock arlock(lock);
            Packet header(PACKET_ID_READER);
            header.SourceKey(readerkey).DestinationKey(writerkey);
            SendPacket(header);
        }

        void SendIDWriter() {
            Sync::AutoReentrantLock arlock(lock);
            Packet header(PACKET_ID_WRITER);
            header.SourceKey(writerkey).DestinationKey(readerkey);
            SendPacket(header);
        }

        void InitiateConnection() {
            Sync::AutoReentrantLock arlock(lock);
            FUNC_TRACE(kch.logger);
            shared_ptr<Database> database = kch.kmh->GetDatabase();
            Key_t hostkey;
            if (mode == ID_READER) {
                hostkey = database->GetWriterHost(writerkey);
            } else if (mode == ID_WRITER) {
                hostkey = database->GetReaderHost(readerkey);
            } else {
                ASSERT(false, "Can't initiate a connection from current state.");
            }

            std::string hostname, servname;
            database->GetHostConnectionInfo(hostkey, hostname, servname);
            SockAddrList addrlist = SocketAddress::CreateIP(hostname, servname);
            SockHandler::Connect(addrlist);
            if (mode == ID_READER) {
                SendIDReader();
            } else if (mode == ID_WRITER) {
                SendIDWriter();
            }
            mode = WAITING;
        }

        void LogState() {
            switch (mode) {
            case ID_READER:
                kch.logger.Debug("Connection in mode ID_READER w:%lu r:%lu", writerkey, readerkey);
                break;
            case ID_WRITER:
                kch.logger.Debug("Connection in mode ID_WRITER w:%lu r:%lu", writerkey, readerkey);
                break;
            case WAITING:
                kch.logger.Debug("Connection in mode WAITING w:%lu r:%lu", writerkey, readerkey);
                break;
            case UNKNOWN:
                kch.logger.Debug("Connection in mode UNKNOWN");
                break;
            case DEAD:
                kch.logger.Debug("Connection dead");
                break;
            }
            if (Closed()) {
                kch.logger.Debug("With no FD");
            }
        }
    private:
        KernelConnectionHandler &kch;
        Sync::ReentrantLock &lock;
        Mode_t mode;
        Key_t readerkey;
        Key_t writerkey;
    };

    KernelConnectionHandler::KernelConnectionHandler(KernelMessageHandler *kmh_)
        : kmh(kmh_)
    {
        Readable(true);
    }

    void KernelConnectionHandler::OnRead() {
        Sync::AutoReentrantLock arlock(lock);
        FUNC_TRACE(logger);
        try {
            SocketAddress addr;
            int newfd = Accept(addr);
            if (newfd >= 0) {
                connlist.push_back(shared_ptr<Connection>(new Connection(*this, lock, Connection::UNKNOWN)));
                connlist.back()->FD(newfd);
            }
        } catch (const ErrnoException &e) {
            logger.Error("Error accepting connection(%d): %s", e.Error(), e.what());
        }
    }

    void KernelConnectionHandler::OnError() {
        Sync::AutoReentrantLock arlock(lock);
        logger.Error("An error occured on the kernel listen socket.");
    }

    void KernelConnectionHandler::OnInval() {
        Sync::AutoReentrantLock arlock(lock);
        logger.Error("The kernel listen socket is invalid!?!?");
    }

    void KernelConnectionHandler::Register(std::vector<FileHandler*> &filehandlers) {
        Sync::AutoReentrantLock arlock(lock);
        if (!Closed()) {
            filehandlers.push_back(this);
        }
        ConnList::iterator itr = connlist.begin();
        while (itr != connlist.end()) {
            if (itr->get()->Closed() || itr->get()->Dead()) {
                itr = connlist.erase(itr);
            } else {
                filehandlers.push_back(itr->get());
                ++itr;
            }
        }
        ConnMap::iterator entry = connmap.begin();
        while (entry != connmap.end()) {
            shared_ptr<Connection> conn = entry->second;
            if (conn->Closed()) {
                if (conn->Dead()) {
                    ConnMap::iterator toerase = entry;
                    ++entry;
                    connmap.erase(toerase);
                } else {
                    if (conn->Mode() == Connection::ID_WRITER) {
                        try {
                            conn->InitiateConnection();
                            filehandlers.push_back(conn.get());
                        } catch (const ErrnoException &e) {
                            logger.Error("Error initiating connection (%d): %s", e.Error(), e.what());
                        }
                    }
                    ++entry;
                }
            } else {
                filehandlers.push_back(conn.get());
                ++entry;
            }
        }
    }

    void KernelConnectionHandler::Shutdown() {
        Sync::AutoReentrantLock arlock(lock);
        FUNC_TRACE(logger);
        Close();
        connlist.clear();
        connmap.clear();
    }

    shared_ptr<Future<int> > KernelConnectionHandler::GetReaderDescriptor(Key_t readerkey, Key_t writerkey) {
        Sync::AutoReentrantLock arlock(lock);
        FUNC_TRACE(logger);
        shared_ptr<Connection> conn;
        ConnMap::iterator entry = connmap.find(readerkey);
        if (entry == connmap.end()) {
            conn = shared_ptr<Connection>(new Connection(*this, lock, Connection::ID_READER));
            conn->WriterKey(writerkey);
            conn->ReaderKey(readerkey);
            connmap.insert(std::make_pair(readerkey, conn));
        } else {
            conn = entry->second;
            if (conn->Dead()) {
                conn->Mode(Connection::ID_READER);
            }
        }
        return conn;
    }

    shared_ptr<Future<int> > KernelConnectionHandler::GetWriterDescriptor(Key_t readerkey, Key_t writerkey) {
        Sync::AutoReentrantLock arlock(lock);
        FUNC_TRACE(logger);
        shared_ptr<Connection> conn;
        ConnMap::iterator entry = connmap.find(writerkey);
        if (entry == connmap.end()) {
            conn = shared_ptr<Connection>(new Connection(*this, lock, Connection::ID_WRITER));
            conn->WriterKey(writerkey);
            conn->ReaderKey(readerkey);
            connmap.insert(std::make_pair(writerkey, conn));
        } else {
            conn = entry->second;
            if (conn->Dead()) {
                conn->Mode(Connection::ID_WRITER);
            }
        }
        kmh->SendWakeup();
        return conn;
    }

    void KernelConnectionHandler::Transfer(Key_t key, shared_ptr<Connection> conn) {
        Sync::AutoReentrantLock arlock(lock);
        FUNC_TRACE(logger);
        ConnMap::iterator entry = connmap.find(key);
        if (entry == connmap.end()) {
            connmap.insert(std::make_pair(key, conn));
        } else {
            entry->second->Transfer(conn);
        }
    }

    void KernelConnectionHandler::SetupLogger() {
        Sync::AutoReentrantLock arlock(lock);
        logger.Name("ConnHan");
        logger.Output(kmh->GetLogger());
        logger.LogLevel(kmh->GetLogger()->LogLevel());
    }

    void KernelConnectionHandler::LogState() {
        if (Closed()) {
            logger.Debug("Listener closed!");
        }
        logger.Debug("Printing list:");
        ConnList::iterator itr = connlist.begin();
        while (itr != connlist.end()) {
            logger.Debug("Conn %p:", itr->get());
            itr->get()->LogState();
            ++itr;
        }
        logger.Debug("Printing map:");
        ConnMap::iterator entry = connmap.begin();
        while (entry != connmap.end()) {
            logger.Debug("Conn %p:", entry->second.get());
            entry->second->LogState();
            ++entry;
        }
    }

}

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
#include "SockHandler.h"
#include "PacketDecoder.h"
#include "PacketEncoder.h"
#include "Assert.h"
#include "ErrnoException.h"

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
            ID_WRITER
        };

        Connection(KernelConnectionHandler &kch_, Sync::ReentrantLock &lock_, Mode_t mode_)
            : kch(kch_), lock(lock_), mode(mode_), readerkey(0), writerkey(0)
        {
            Writeable(false);
            kch.logger.Info("New Connection(%d)", mode);
        }

        // Future<int>
        virtual bool Done() {
            Sync::AutoReentrantLock arlock(lock);
            return mode == WAITING;
        }

        virtual void Cancel() {
            Sync::AutoReentrantLock arlock(lock);
            Close();
        }

        virtual int Get() {
            Sync::AutoReentrantLock arlock(lock);
            int ret = FD();
            FileHandler::Reset();
            return ret;
        }

        // FileHandler
        virtual void OnRead() {
            Sync::AutoReentrantLock arlock(lock);
            if (!Good()) { return; }
            ASSERT(!Done());
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
                mode = WAITING;
                Close();
            }
        }

        virtual void OnWrite() {}
        virtual void OnError() {
            Sync::AutoReentrantLock arlock(lock);
            mode = WAITING;
            Close();
        }
        virtual void OnHup() {
            Sync::AutoReentrantLock arlock(lock);
            mode = WAITING;
            Close();
        }
        virtual void OnInval() {
            Sync::AutoReentrantLock arlock(lock);
            mode = WAITING;
            FileHandler::Reset();
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
            readerkey = packet.SourceKey();
            writerkey = packet.DestinationKey();
            mode = WAITING;
            kch.Transfer(writerkey, shared_from_this());
        }
        virtual void IDWriterPacket(const Packet &packet) {
            readerkey = packet.SourceKey();
            writerkey = packet.DestinationKey();
            mode = WAITING;
            kch.Transfer(readerkey, shared_from_this());
        }
        // PacketEncoder
        virtual void WriteBytes(const iovec *iov, unsigned iovcnt) {
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
            ASSERT(Closed());
            mode = conn->Mode();
            ASSERT(readerkey == conn->ReaderKey());
            ASSERT(writerkey == conn->WriterKey());
            FD(conn->Get());
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
        logger.Output(kmh->GetLogger());
        logger.Name("ConnHan");
    }

    void KernelConnectionHandler::OnRead() {
        Sync::AutoReentrantLock arlock(lock);
        SocketAddress addr;
        int newfd = Accept(addr);
        if (newfd >= 0) {
            connlist.push_back(shared_ptr<Connection>(new Connection(*this, lock, Connection::UNKNOWN)));
            connlist.back()->FD(newfd);
        }
    }

    void KernelConnectionHandler::OnError() {
        Sync::AutoReentrantLock arlock(lock);
        logger.Error("An error occured on the kernel listen socket.");
    }

    void KernelConnectionHandler::OnInval() {
        Sync::AutoReentrantLock arlock(lock);
        logger.Error("The kernel listen socke is invalid!?!?");
    }

    void KernelConnectionHandler::Register(std::vector<FileHandler*> &filehandlers) {
        Sync::AutoReentrantLock arlock(lock);
        if (!Closed()) {
            filehandlers.push_back(this);
        }
        ConnList::iterator itr = connlist.begin();
        while (itr != connlist.end()) {
            if (itr->get()->Closed() || itr->get()->Done()) {
                itr = connlist.erase(itr);
            } else {
                filehandlers.push_back(itr->get());
                ++itr;
            }
        }
        ConnMap::iterator entry = connmap.begin();
        while (entry != connmap.end()) {
            if (entry->second->Closed()) {
                ConnMap::iterator toerase = entry;
                ++entry;
                connmap.erase(toerase);
            } else {
                filehandlers.push_back(entry->second.get());
                ++entry;
            }
        }
    }

    void KernelConnectionHandler::Shutdown() {
        Sync::AutoReentrantLock arlock(lock);
        Close();
        connlist.clear();
        connmap.clear();
    }

    shared_ptr<Future<int> > KernelConnectionHandler::GetReaderDescriptor(Key_t readerkey, Key_t writerkey) {
        Sync::AutoReentrantLock arlock(lock);
        shared_ptr<Connection> conn;
        ConnMap::iterator entry = connmap.find(readerkey);
        if (entry == connmap.end()) {
            conn = shared_ptr<Connection>(new Connection(*this, lock, Connection::ID_READER));
            conn->WriterKey(writerkey);
            conn->ReaderKey(readerkey);
            connmap.insert(std::make_pair(readerkey, conn));
        }
        return conn;
    }

    shared_ptr<Future<int> > KernelConnectionHandler::GetWriterDescriptor(Key_t readerkey, Key_t writerkey) {
        Sync::AutoReentrantLock arlock(lock);
        shared_ptr<Connection> conn;
        ConnMap::iterator entry = connmap.find(writerkey);
        if (entry == connmap.end()) {
            conn = shared_ptr<Connection>(new Connection(*this, lock, Connection::ID_WRITER));
            conn->WriterKey(writerkey);
            conn->ReaderKey(readerkey);
            connmap.insert(std::make_pair(writerkey, conn));
        }
        return conn;
    }

    void KernelConnectionHandler::Transfer(Key_t key, shared_ptr<Connection> conn) {
        connlist.erase(std::find(connlist.begin(), connlist.end(), conn));
        ConnMap::iterator entry = connmap.find(key);
        if (entry == connmap.end()) {
            connmap.insert(std::make_pair(key, conn));
        } else {
            entry->second->Transfer(conn);
        }
    }

}

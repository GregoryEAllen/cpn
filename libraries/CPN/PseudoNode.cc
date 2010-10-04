
#include "PseudoNode.h"
#include "D4RNode.h"
#include "Database.h"
#include "QueueReader.h"
#include "QueueWriter.h"


namespace CPN {

    PseudoNode::PseudoNode(const std::string &n, Key_t k, shared_ptr<Database> db)
        : name(n),
        nodekey(k),
        d4rnode(new D4R::Node(k)),
        database(db)
    {
    }

    PseudoNode::~PseudoNode() {
    }

    shared_ptr<QueueReader> PseudoNode::GetReader(const std::string &portname) {
        database->CheckTerminated();
        Key_t ekey = database->GetCreateReaderKey(nodekey, portname);
        return GetReader(ekey);
    }

    shared_ptr<QueueWriter> PseudoNode::GetWriter(const std::string &portname) {
        database->CheckTerminated();
        Key_t ekey = database->GetCreateWriterKey(nodekey, portname);
        return GetWriter(ekey);
    }

    void PseudoNode::CreateReader(shared_ptr<QueueBase> q) {
        Sync::AutoReentrantLock arl(lock);
        Key_t readerkey = q->GetReaderKey();
        d4rnode->AddReader(q);
        q->SetReaderNode(d4rnode);
        q->SignalReaderTagChanged();
        ASSERT(readermap.find(readerkey) == readermap.end(), "The reader already exists");
        shared_ptr<QueueReader> reader;
        reader = shared_ptr<QueueReader>(new QueueReader(this, q));
        readermap.insert(std::make_pair(readerkey, reader));
        cond.Signal();
    }

    void PseudoNode::CreateWriter(shared_ptr<QueueBase> q) {
        Sync::AutoReentrantLock arl(lock);
        Key_t writerkey = q->GetWriterKey();
        d4rnode->AddWriter(q);
        q->SetWriterNode(d4rnode);
        q->SignalWriterTagChanged();
        ASSERT(writermap.find(writerkey) == writermap.end(), "The writer already exists.");
        shared_ptr<QueueWriter> writer;
        writer = shared_ptr<QueueWriter>(new QueueWriter(this, q));
        writermap.insert(std::make_pair(writerkey, writer));
        cond.Signal();
    }

    void PseudoNode::Shutdown() {
        Sync::AutoReentrantLock arl(lock);
        ReaderMap readers;
        readers.swap(readermap);
        WriterMap writers;
        writers.swap(writermap);
        arl.Unlock();
        readers.clear();
        writers.clear();

    }

    void PseudoNode::ReleaseReader(Key_t ekey) {
        shared_ptr<QueueReader> reader;
        Sync::AutoReentrantLock arl(lock);
        ReaderMap::iterator entry = readermap.find(ekey);
        if (entry != readermap.end()) {
            reader = entry->second;
            readermap.erase(entry);
        }
        arl.Unlock();
        reader.reset();
    }

    void PseudoNode::ReleaseWriter(Key_t ekey) {
        shared_ptr<QueueWriter> writer;
        Sync::AutoReentrantLock arl(lock);
        WriterMap::iterator entry = writermap.find(ekey);
        if (entry != writermap.end()) {
            writer = entry->second;
            writermap.erase(entry);
        }
        arl.Unlock();
        writer.reset();
    }

    void PseudoNode::NotifyTerminate() {
        Sync::AutoReentrantLock arl(lock);
        cond.Signal();
        WriterMap::iterator witr = writermap.begin();
        while (witr != writermap.end()) { (witr++)->second->NotifyTerminate(); }
        ReaderMap::iterator ritr = readermap.begin();
        while (ritr != readermap.end()) { (ritr++)->second->NotifyTerminate(); }
    }

    shared_ptr<QueueReader> PseudoNode::GetReader(Key_t ekey) {
        Sync::AutoReentrantLock arl(lock);
        shared_ptr<QueueReader> reader;
        while (!reader) {
            ReaderMap::iterator entry = readermap.find(ekey);
            if (entry == readermap.end()) {
                database->CheckTerminated();
                cond.Wait(lock);
            } else {
                reader = shared_ptr<QueueReader>(entry->second);
            }
        }
        return reader;
    }

    shared_ptr<QueueWriter> PseudoNode::GetWriter(Key_t ekey) {
        Sync::AutoReentrantLock arl(lock);
        shared_ptr<QueueWriter> writer;
        while (!writer) {
            WriterMap::iterator entry = writermap.find(ekey);
            if (entry == writermap.end()) {
                database->CheckTerminated();
                cond.Wait(lock);
            } else {
                writer = shared_ptr<QueueWriter>(entry->second);
            }
        }
        return writer;

    }

    bool PseudoNode::IsPurePseudo() {
        return true;
    }

    void PseudoNode::LogState() {
    }
}


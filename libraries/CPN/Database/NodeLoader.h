#pragma once
#include "CPNCommon.h"
#include "PthreadMutex.h"
#include <string>
#include <map>
#include <vector>

#define CPN_DEFAULT_INIT_SYMBOL cpninit
#define CPN_DEFAULT_INIT_SYMBOL_STR "cpninit"
namespace CPN {
    /** \brief This is the prototype of the function
     * that is called by the dynamic library loading
     * facility.
     */
    typedef shared_ptr<NodeFactory> (*CPNInitPrototype)(void);


    class NodeLoader {
    public:
        NodeLoader();
        ~NodeLoader();
        void LoadSharedLib(const std::string &libname);
        void LoadNodeList(const std::string &filename);

        NodeFactory *GetFactory(const std::string &nodename);
        void RegisterFactory(shared_ptr<NodeFactory> factory);

    private:
        void InternalLoadLib(const std::string &lib);
        void InternalLoad(const std::string &sym);

        PthreadMutex lock;
        typedef std::map<std::string, void*> LibMap;
        LibMap libmap;
        typedef std::map<std::string, shared_ptr<NodeFactory> > FactoryMap;
        FactoryMap factorymap;
        typedef std::map<std::string, std::string> NodeLibMap;
        NodeLibMap nodelibmap;

    };
}

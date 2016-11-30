/*
 * SelectSocketSet.h
 *
 *  Created on: Nov 04, 2016
 *      Author: j.zhou
 */

#ifndef SELECT_SOCKET_SET_H
#define SELECT_SOCKET_SET_H

#include <map>
#include <list>

#include "SocketSetInterface.h"
#include "MutexLock.h"
#include "NetLogger.h"

namespace net {    

    class SelectSocketSet : public SocketSetInterface {
    public:
        SelectSocketSet();
        virtual ~SelectSocketSet();

        // Listen on all registered socket and return all available SelectSocket
        //  objects or return one abject with events set to 0 after timeout
        //
        // Arguments:
        //  theTimeout - the select timeout in millisecond
        // 
        // Return: void* -> SelectSocket*
        // 
        // Exception: N/A
        virtual void* poll(int theTimeout);

        virtual void registerInputHandler(Socket* theSocket, SocketEventHandler* theEventHandler);
        virtual void removeInputHandler(Socket* theSocket);
        virtual void registerOutputHandler(Socket* theSocket, SocketEventHandler* theEventHandler);
        virtual void removeOutputHandler(Socket* theSocket);
        virtual void removeHandlers(Socket* theSocket);

        virtual int getNumberOfSocket() const; 

        enum {
            MAX_FD_SIZE_FOR_SELECT = 1024
        };

        enum {
            SELECT_R_EV = 1,
            SELECT_W_EV = 2
        };

        enum {
            SELECT_ADD_EV,
            SELECT_DEL_EV
        };

        struct SelectSocket {
            Socket* socket;
            SocketEventHandler* eventHandler;
            int events;
        };

        struct UpdateSocket {
            int fd;
            int op;
            int events;
        };

    private:
        void registerSocketEvent(Socket* theSocket, SocketEventHandler* theEventHandler, int event);
        void updateEvents();

        typedef std::map<int, SelectSocket> SelectSocketMap;  
        SelectSocketMap m_selectSocketMap;

        // select fd
        fd_set m_rFdSet;    // read fd set
        fd_set m_wFdSet;    // write fd set

        // socket array that ready to read or write
        SelectSocket* m_readySocketArray;

        // use recursive mutex
        cm::MutexLock* m_lock;

        // number of events become available 
        int m_numFds;

        // fd to select
        int m_selectFd;        

        typedef std::list<UpdateSocket> UpdateSocketList;
        UpdateSocketList m_updateSocketList;
    };

    // ----------------------------------------------------
    inline void SelectSocketSet::registerInputHandler(
        Socket* theSocket, 
        SocketEventHandler* theEventHandler) 
    {
        registerSocketEvent(theSocket, theEventHandler, SELECT_R_EV);
    }

    // ----------------------------------------------------
    inline void SelectSocketSet::removeInputHandler(Socket* theSocket) {
        //TODO
    }

    // ----------------------------------------------------
    inline void SelectSocketSet::registerOutputHandler(
        Socket* theSocket, 
        SocketEventHandler* theEventHandler) 
    {
        //TODO
    }

    // ----------------------------------------------------
    inline void SelectSocketSet::removeOutputHandler(Socket* theSocket) {
        //TODO
    }

    // ----------------------------------------------------
    inline void SelectSocketSet::removeHandlers(Socket* theSocket) {
        //TODO
    }

    // ----------------------------------------------------
    inline int SelectSocketSet::getNumberOfSocket() const {
        //TODO
        return 0;
    }

    // ----------------------------------------------------
    inline void SelectSocketSet::registerSocketEvent(
        Socket* theSocket, 
        SocketEventHandler* theEventHandler, 
        int event) 
    {
        if (m_selectSocketMap.size() >= MAX_FD_SIZE_FOR_SELECT) {
            LOG4CPLUS_WARN(_NET_LOOGER_NAME_, "select can not support fd number more than " << MAX_FD_SIZE_FOR_SELECT);
            return;
        }

        int fd = theSocket->getSocket();

        SelectSocket selectSocket;
        selectSocket.socket = theSocket;
        selectSocket.eventHandler = theEventHandler;
        selectSocket.events = event;

        m_lock->lock();

        std::pair<SelectSocketMap::iterator, bool> result = 
            m_selectSocketMap.insert(SelectSocketMap::value_type(fd, selectSocket));
        if (!result.second) {
            // The socket is already registered in epoll, update it.
            // Maybe read or write event or both are registered, but we just
            // check simply if there is aleast one event not registered, then 
            // re-register all new events again
            if ((result.first)->second.events != selectSocket.events) {
                (result.first)->second.events |= selectSocket.events;
                (result.first)->second.socket = selectSocket.socket;
                (result.first)->second.eventHandler = selectSocket.eventHandler;
            } else {
                // if the events is already register
                m_lock->unlock();  
                return;
            }            
        }

        UpdateSocket updateSocket;
        updateSocket.fd = fd;
        updateSocket.op = SELECT_ADD_EV;
        updateSocket.events = selectSocket.events;
        m_updateSocketList.push_back(updateSocket);

        m_lock->unlock();  
    }

}

#endif

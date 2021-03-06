/******************************************************************************/
/*
  Author  - Ming-Lun "Allen" Chou
  Web     - http:// AllenChou.net
  Twitter - @TheAllenChou
 */
/******************************************************************************/

#include "client.h"

namespace sidnet
{
  DWORD WINAPI ClientReceiveMain(void *data)
  {
    ClientReceiveMainData *pCrmData = static_cast<ClientReceiveMainData *>(data);
    Client *pClient = pCrmData->m_pClient;
    delete pCrmData; // no longer needed
    
    size_t messageSize = 256;
    char *buffer = new char[messageSize];
    
    
    // main receive loop
    // TODO: find a way to stop client
    while (1)
    {
      size_t charsToReceive = 0;
      size_t dataSize = 0;
      
      // receive size_t portion
      dataSize = charsToReceive = sizeof(size_t);
      while (charsToReceive)
      {
        int err = pClient->m_pSocket->Read(buffer + (dataSize - charsToReceive), charsToReceive);
        
        if (err == SOCKET_ERROR)
        {
          int err = WSAGetLastError();
          if (err == WSAEWOULDBLOCK)
          {
            // non-blocking (not an error)
            Sleep(1);
            continue;
          }
          else
          {
            // error, shut down
            pClient->m_pSocket->ShutDown();
            return err;
          }
        }
        else if (err == 0)
        {
          // connection ended
          return pClient->m_pSocket->ShutDown();
        }
        
        // decrement chars to receive
        charsToReceive -= err;
        Sleep(1);
      }
      
      
      // deserialize size_t
      size_t deserializedMessageSize = 0;
      for (size_t i = 0; i < sizeof(size_t); ++i)
      {
        deserializedMessageSize |= static_cast<size_t>(buffer[i]) << (8 * (sizeof(size_t) - 1 - i));
      }
      

      // buffer not large enough, double each iteration
      if (deserializedMessageSize > messageSize)
      {
        delete buffer;
        
        do 
        {
          messageSize <<= 1;
        } while (deserializedMessageSize > messageSize);
        
        buffer = new char[messageSize];
      }
      
      
      // receive string portion
      dataSize = charsToReceive = deserializedMessageSize;
      while (charsToReceive)
      {
        int err = pClient->m_pSocket->Read(buffer + (dataSize - charsToReceive), charsToReceive);
        
        if (err == SOCKET_ERROR)
        {
          int err = WSAGetLastError();
          if (err == WSAEWOULDBLOCK)
          {
            // non-blocking (not an error)
            Sleep(1);
            continue;
          }
          else
          {
            // error, shut down
            pClient->m_pSocket->ShutDown();
            return err;
          }
        }
        else if (err == 0)
        {
          // connection ended
          return pClient->m_pSocket->ShutDown();
        }

        // decrement chars to receive
        charsToReceive -= err;
        Sleep(1);
      }
      
      
      // sufficient buffer, call Client::OnReceive
      int err = pClient->OnReceive(buffer, dataSize);
      if (err)
      {
        // shut down
        pClient->m_pSocket->ShutDown();
        break;
      }
    }
    
    return 0;
  }
  
  
  
  Client::Client()
    : m_pSocket(nullptr)
  {
    InitializeCriticalSection(&m_socketCriticalSection);
  }
  
  Client::~Client()
  {
    if (m_pSocket) delete m_pSocket;
    DeleteCriticalSection(&m_socketCriticalSection);
  }

  int Client::Connect(const char *ipAddress, const unsigned short port)
  {
    m_pSocket = new Socket();
    int err = m_pSocket->Connect(ipAddress, port);
    if (err == 0)
    {
      ClientReceiveMainData *data = new ClientReceiveMainData;
      data->m_pClient = this;
      
      // create client receive main thread
      CreateThread(0, 0, ClientReceiveMain, static_cast<void *>(data), 0, 0);
    }
    
    return err;
  }
  
  int Client::ShutDown()
  {
    if (m_pSocket) return m_pSocket->ShutDown();
    return 0;
  }

  int Client::OnReceive(const char *buffer, size_t size)
  {
    return 0;
  }
  
  int Client::Send(const char *buffer, size_t size)
  {
    // serialize size_t
    char sizeBuffer[sizeof(size_t)];
    size_t deserializedMessageSize = 0;
    for (size_t i = 0; i < sizeof(size_t); ++i)
    {
      sizeBuffer[i] = static_cast<char>((size >> (8 * (sizeof(size_t) - 1 - i))) & 0xFF);
    }
    
    size_t charsToSend = 0;
    
    // send string size
    charsToSend = sizeof(size_t);
    while (charsToSend)
    {
      int err = m_pSocket->Send(sizeBuffer + (sizeof(size_t) - charsToSend), charsToSend);
        
      if (err == SOCKET_ERROR)
      {
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK)
        {
          // non-blocking (not an error)
          Sleep(1);
          continue;
        }
        else
        {
          // error, shut down
          m_pSocket->ShutDown();
          return err;
        }
      }
      charsToSend -= err;
      Sleep(1);
    }
    
    // send string
    charsToSend = size;
    while (charsToSend)
    {
      int err = m_pSocket->Send(buffer + (size - charsToSend), charsToSend);
      
      if (err == SOCKET_ERROR)
      {
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK)
        {
          // non-blocking (not an error)
          Sleep(1);
          continue;
        }
        else
        {
          // error, shut down
          m_pSocket->ShutDown();
          return err;
        }
      }
      charsToSend -= err;
      Sleep(1);
    }
    
    return 0;
  }
  
}
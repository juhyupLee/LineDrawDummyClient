#include <iostream>
#include "RingBuffer.h"
RingBuffer::RingBuffer()
    :
    m_Buffer{ 0, },
    m_Front(0),
    m_Rear(0),
    m_UsedSize(0),
    m_FreeSize(RING_BUFFER_SIZE-1)
{
}

RingBuffer::~RingBuffer()
{
}

int RingBuffer::Enqueue(char* buffer, int32_t size)
{
    //-----------------------------------
    // 링버퍼의 남은 사이즈가 들어온 사이즈보다 작다면 size를 프리사이즈로바꿈
    //-----------------------------------
    if (m_FreeSize < size)
    {
        size = m_FreeSize;
    }

    int diff = 0;
    int directSize = GetDirectEnqueueSize();
    //-----------------------------------
    // Enqueue 들어오면 m_Rear는 쁠쁠 먼저하고, Enqeue를한다.
    //-----------------------------------
    m_Rear = (m_Rear + 1) % RING_BUFFER_SIZE;

    //-----------------------------------
    // 인자로 들어온 사이즈가  연속된 사이즈 보다 더 크다면, memcpy를 나눠서해야한다
    //-----------------------------------
    if (directSize < size)
    {
        memcpy(m_Buffer + m_Rear, buffer, directSize);
        m_Rear = (m_Rear +directSize-1) % RING_BUFFER_SIZE;
        diff = size - directSize;

        m_Rear = (m_Rear + 1) % RING_BUFFER_SIZE;
        memcpy(m_Buffer + m_Rear, buffer + directSize, diff);
        m_Rear = (m_Rear+ diff-1) % RING_BUFFER_SIZE;
    }
    else
    {
        memcpy(m_Buffer + m_Rear, buffer, size);
        m_Rear = (m_Rear +size-1) % RING_BUFFER_SIZE;
    }

    m_UsedSize += size;
    m_FreeSize -= size;
    return size;
}

int RingBuffer::Dequeue(char* buffer, int32_t size)
{

    //-------------------------------------------
    //링버퍼에 사용중인 사이즈가 인자로 들어온 사이즈보다 작으면 실패
    //-------------------------------------------
    if (m_UsedSize < size)
    {
        size = m_UsedSize;
    }
    int directSize = GetDirectDequeueSize();

    //-----------------------------------
    // Enqueue 들어오면 m_Rear는 쁠쁠 먼저하고, Enqeue를한다.
    //-----------------------------------
    m_Front = (m_Front + 1) % RING_BUFFER_SIZE;
    //-------------------------------------------
    // 연속할당할수있는 사이즈가 들어온 사이즈보다 작다면 두번나눠서 디큐를 해야함.
    //-------------------------------------------
    if (directSize < size)
    {
        memcpy(buffer, m_Buffer + m_Front, directSize);
        m_Front = (m_Front + directSize -1) % RING_BUFFER_SIZE;;

        int diff = size - directSize;
        m_Front = (m_Front + 1) % RING_BUFFER_SIZE;
        memcpy(buffer + directSize, m_Buffer + m_Front, diff);
        m_Front = (m_Front+diff -1 ) % RING_BUFFER_SIZE;
    }
    else
    {
        memcpy(buffer, m_Buffer + m_Front, size);
        m_Front = (m_Front+size -1 ) % RING_BUFFER_SIZE;
    }
    m_UsedSize -= size;
    m_FreeSize += size;
    return size;
}

int32_t RingBuffer::GetFreeSize() const
{
    return m_FreeSize;
}

int32_t RingBuffer::GetUsedSize() const
{
    return m_UsedSize;
}

int32_t RingBuffer::GetDirectEnqueueSize() const
{

     //-----------------------------------
    // 링버퍼의 사이즈가 꽉찼다면, Direct Size를 구할수 없다. 0반환
    //-----------------------------------
    if ((m_Rear + 1) % RING_BUFFER_SIZE == m_Front)
    {
        return 0;
    }

    int directSize = 0;

    int rearNext = (m_Rear + 1) % RING_BUFFER_SIZE;

    //-----------------------------------
    // mRear < mFront < 인덱스 끝
    // 이러면  연속된 인덱스는 m_Front 전까지다.
    //-----------------------------------

    if (rearNext < m_Front)
    {
        directSize = m_Front - rearNext;
    }
    //-----------------------------------
    // 그것이 아니라면, m_Rear ~ 인덱스끝까지가 연속된 메모리 할당범위다.
    //-----------------------------------
    else
    {
        directSize = (RING_BUFFER_SIZE - 1) - rearNext + 1;
    }

    if (directSize == 0)
    {
        int a = 10;
    }

    return directSize;
}

int32_t RingBuffer::GetDirectDequeueSize() const
{
    //------------------------------------------
    // 링버퍼가 비어있다면, 0을반환
    //------------------------------------------
    if (m_Front == m_Rear)
    {
        return 0;
    }


    int nextFront = (m_Front + 1) % RING_BUFFER_SIZE;

    int32_t directSize = 0;
    //일반적인 경우
    if (nextFront < m_Rear)
    {
        directSize = m_Rear - nextFront+1;
    }
    else
    {
        directSize = (RING_BUFFER_SIZE - 1) - nextFront+1;
    }

    return directSize;

}

void RingBuffer::ClearBuffer()
{
    m_FreeSize = RING_BUFFER_SIZE;
    m_UsedSize = 0;
    m_Front = 0;
    m_Rear = 0;
}

void RingBuffer::MoveRear(int size)
{
    if (size < 0)
    {
        return;
    }
    m_Rear = (m_Rear + size) % RING_BUFFER_SIZE;
    m_UsedSize += size;
    m_FreeSize -= size;
}

void RingBuffer::MoveFront(int size)
{
    if (size < 0)
    {
        return;
    }
    //m_Front = (m_Front + 1) % RING_BUFFER_SIZE;;
    m_Front = (m_Front + size ) % RING_BUFFER_SIZE;
    m_UsedSize -= size;
    m_FreeSize += size;
}

char* RingBuffer::GetFrontBufferPtr(void)
{
    //--------------------------------------
    // FrontBufferPtr을 쓰려고하는것은 지금
    // dequeue를하려는 상황인데, 큐가 비어있다면
    // null을반환
    //--------------------------------------
    if (m_Front == m_Rear)
    {
        return nullptr;
    }

    return m_Buffer + ((m_Front+1)%RING_BUFFER_SIZE);
}

char* RingBuffer::GetRearBufferPtr(void)
{
    //--------------------------------------
    // Enqueue를하려는 상황인데, 큐가 꽉차있다면,
    // null을반환
    //--------------------------------------
    if ((m_Rear+1)%RING_BUFFER_SIZE == m_Front)
    {
        return nullptr;
    }

    return m_Buffer + ((m_Rear + 1) % RING_BUFFER_SIZE); 
}

int RingBuffer::Peek(char* dest, int size)
{
    //-------------------------------------------
   //링버퍼에 사용중인 사이즈가 인자로 들어온 사이즈보다 작으면 현재 사용중인 사이즈로바꿈.
   //-------------------------------------------
    if (m_UsedSize < size)
    {
        size = m_UsedSize;
    }
    int directSize = GetDirectDequeueSize();

    //-----------------------------------
    // Peek이기때문에 프론트 사본 만듬.
    //-----------------------------------
    int tempFront = m_Front;

    //-----------------------------------
    // Enqueue 들어오면 m_Rear는 쁠쁠 먼저하고, Enqeue를한다.
    //-----------------------------------
    
    tempFront = (tempFront + 1) % RING_BUFFER_SIZE;
    //-------------------------------------------
    // 연속할당할수있는 사이즈가 들어온 사이즈보다 작다면 두번나눠서 디큐를 해야함.
    //-------------------------------------------
    if (directSize < size)
    {
        memcpy(dest, m_Buffer + tempFront, directSize);
        tempFront = (tempFront + directSize - 1) % RING_BUFFER_SIZE;;

        int diff = size - directSize;
        tempFront = (tempFront + 1) % RING_BUFFER_SIZE;
        memcpy(dest + directSize, m_Buffer + tempFront, diff);
    }
    else
    {
        memcpy(dest, m_Buffer + tempFront, size);
    }

    return size;
}

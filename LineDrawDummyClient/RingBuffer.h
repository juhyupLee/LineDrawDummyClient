#pragma once
class RingBuffer
{

public:
	enum
	{
		RING_BUFFER_SIZE = 19
	};

public:
	RingBuffer();
	~RingBuffer();
public:
	int Enqueue(char* buffer, int32_t size);
	int Dequeue(char* buffer, int32_t size);
	//bool Enqueue(char* buffer, int32_t size);
	//bool Dequeue(char* buffer, int32_t size);
	

	int32_t GetFreeSize() const; 
	int32_t GetUsedSize() const;

	int32_t GetDirectEnqueueSize()const;
	int32_t GetDirectDequeueSize()const;
	void ClearBuffer();

	void MoveRear(int size);
	void MoveFront(int size);
	char* GetFrontBufferPtr(void);
	char* GetRearBufferPtr(void);
	int Peek(char* dest, int size);

private:
	int32_t m_Front;
	int32_t m_Rear;
	int32_t m_FreeSize;
	int32_t m_UsedSize;
	char m_Buffer[RING_BUFFER_SIZE];

};
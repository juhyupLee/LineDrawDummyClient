#pragma once
class CLogManager
{
private:
	CLogManager();
	~CLogManager();
public:
	enum
	{
		FILE_NAME = 128
	};
	static CLogManager* GetInstance();
	void PrintLog(WCHAR* str);
	void PrintLog(const WCHAR* fileName,const WCHAR* str, __int32 var);
	void PrintLog(const WCHAR* fileName,const WCHAR* str, __int64 var);
	void PrintLog(const WCHAR* fileName,const WCHAR* str, float var);
	void PrintLog(const WCHAR* fileName,const WCHAR* str, double var);
	void PrintLog(const WCHAR* fileName,const WCHAR* str, bool var);

	void PrintLog(const WCHAR* fileName, const WCHAR* str, __int32 var1, __int32 var2);
	void PrintLog(const WCHAR* fileName, const WCHAR* str, __int64 var1, __int64 var2);
	void PrintLog(const WCHAR* fileName, const WCHAR* str, float var1, float var2);
	void PrintLog(const WCHAR* fileName, const WCHAR* str, double var1, double var2);
	void PrintLog(const WCHAR* fileName, const WCHAR* str, bool var1, bool var2);


	void PrintConsoleLog(const WCHAR* str, __int32 var1, __int32 var2);
	void PrintConsoleLog(const WCHAR* str, __int32 var1);

private:
	WCHAR m_FileName[128];
};

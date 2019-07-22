#include <time.h>
#include <sys/stat.h>
#include "log.h"
#include "macro.h"

CMyLog::CMyLog()
	: m_fp(NULL)
	, m_ulFileSize(0)
{
	m_ulFileSize = 1024 * 1024 * 5;
	m_ucLevel = LOG_LEVEL_INFO;

	m_hMutex = CreateMutex(NULL, false, NULL);
	if (!m_hMutex)
	{
		printf("CreateMutex fail\r\n");
	}

	char* home = getenv("HOMEDRIVE");
	assert(home != NULL);
	m_path = std::string(home);
	home = getenv("HOMEPATH");
	assert(home != NULL);
	m_path = m_path + home;

	m_path = m_path + "\\AppData\\Local";
	if (_access(m_path.c_str(), 0))
	{
		printf("HOME\\AppData\\Local dir not exist\n");
		return;
	}

	m_path = m_path + "\\install";
	if (_access(m_path.c_str(), 0))
	{
		_mkdir(m_path.c_str());
	}

	m_path = m_path + "\\ThumbnailHandler";
	if (_access(m_path.c_str(), 0))
	{
		_mkdir(m_path.c_str());
	}

	std::string logfile = m_path + "\\Thumbnail.log";
	m_fp = fopen(logfile.c_str(), "a+");

	if (!m_fp)
	{
		printf("Open log file fail\r\n");
		return;
	}
}

CMyLog::~CMyLog()
{
	if (m_fp)
	{
		fclose(m_fp);
	}

	CloseHandle(m_hMutex);
}

/**
 * ����ʵ��
 */
CMyLog& CMyLog::GetInstance()
{
	static CMyLog* pLog = NULL;

	if (!pLog)
	{
		pLog = new CMyLog;
	}

	return *pLog;
}

void CMyLog::Log(unsigned char level, const wchar_t* file, int line, const wchar_t* fmt, ...)
{
	static wchar_t LogLevelString[][10] = {L"ERROR", L"INFO ", L"DEBUG"};

	if (level > m_ucLevel)
	{
		return;
	}

	WaitForSingleObject(m_hMutex, INFINITE);
	time_t timer = time(NULL);
	struct tm *tmt = localtime(&timer);
	va_list arg;

	/* to log file */
	if (m_fp)
	{
		fwprintf(m_fp, L"%04d-%02d-%02d %02d:%02d:%02d %s ",
			tmt->tm_year + 1900,
			tmt->tm_mon + 1,
			tmt->tm_mday,
			tmt->tm_hour,
			tmt->tm_min,
			tmt->tm_sec,
			LogLevelString[level]);

		/* log content */
		va_start(arg, fmt);
		vfwprintf(m_fp, fmt, arg);
		va_end(arg);

		/* log file */
		fwprintf(m_fp, L" %s:%d\n", file, line);
		fflush(m_fp);
		ChangeLogFile();
	}

	ReleaseMutex(m_hMutex);
}

void CMyLog::Log(unsigned char level, const wchar_t* str)
{
	static wchar_t LogLevelString[][10] = { L"ERROR", L"INFO ", L"DEBUG" };
	if (level > m_ucLevel)
	{
		return;
	}

	WaitForSingleObject(m_hMutex, INFINITE);
	time_t timer = time(NULL);
	struct tm *tmt = localtime(&timer);
	/* to log file */
	if (m_fp)
	{
		fwprintf(m_fp, L"%04d-%02d-%02d %02d:%02d:%02d %s %s\n",
			tmt->tm_year + 1900,
			tmt->tm_mon + 1,
			tmt->tm_mday,
			tmt->tm_hour,
			tmt->tm_min,
			tmt->tm_sec,
			LogLevelString[level],
			str);

		fflush(m_fp);
		ChangeLogFile();
	}

	ReleaseMutex(m_hMutex);
}

/**
 * �ж���־�ļ���С�������������浵
 */
void CMyLog::ChangeLogFile()
{
	std::string origfile = m_path + "/Thumbnail.log";

	// stat()������ȡ�ļ���Ϣ���ж��ļ���С
	struct stat statbuff;
	if (stat(origfile.c_str(), &statbuff) < 0)
	{
		return;
	}

	if (statbuff.st_size  < (int)m_ulFileSize)
	{
		return;
	}

	fclose(m_fp);
	m_fp = NULL;

	char filename[256] = {0};
	time_t timer = time(NULL);
	struct tm *tmt = localtime(&timer);

	CC_SPRINTF(filename, 256 - 1, "/Thumbnail_%04d_%02d_%02d_%02d_%02d_%02d.log",
		tmt->tm_year + 1900,
		tmt->tm_mon + 1,
		tmt->tm_mday,
		tmt->tm_hour,
		tmt->tm_min,
		tmt->tm_sec);

	std::string newfile = m_path + filename;
	// ԭ��־�ļ������ߴ磬������־�ļ����ƴ浵��������־��ԭ��־�ļ����Ʊ���
	rename(origfile.c_str(), newfile.c_str());
	m_fp = fopen(origfile.c_str(), "a+");
	if (!m_fp)
	{
		printf("Failed to open log file.\r\n");
	}
}

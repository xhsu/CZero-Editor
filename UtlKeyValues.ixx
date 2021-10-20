module;

#define _CRT_SECURE_NO_WARNINGS	// This is a very unsafe module.

// C++
#include <string>	// stof

// C
#include <cctype>	// isspace isdigit
#include <cmath>	// roundf
#include <cstdio>	// fopen fclose fread fwrite
#include <cstdlib>	// atoi atof
#include <cstring>	// malloc free strlen

export module UtlKeyValues;

import UtlBuffer;
import UtlString;

export struct NewKeyValues
{
	NewKeyValues(const char* setName)
	{
		m_szName = nullptr;
		m_szValue = nullptr;

		Init();
		SetName(setName);
	}
	virtual ~NewKeyValues(void)
	{
		RemoveEverything();
	}

	// set/get name
	virtual const char* GetName(void)
	{
		if (m_szName)
			return m_szName;

		return "";
	}
	virtual void SetName(const char* setName)
	{
		if (!setName)
			setName = "";

		if (m_szName)
		{
			free(m_szName);
			m_szName = nullptr;
		}

		size_t len = strlen(setName);
		m_szName = (char*)malloc(len + 1);
		strcpy(m_szName, setName);
	}

	// load/save file
	virtual bool LoadFromFile(const char* resourceName)
	{
		FILE* f = fopen(resourceName, "rb");
		if (!f)
			return false;

		int fileSize;

		fseek(f, 0, SEEK_END);
		fileSize = ftell(f);
		fseek(f, 0, SEEK_SET);

		char* buffer = (char*)malloc(fileSize + 1);

		fread(buffer, fileSize, 1, f);

		fclose(f);

		buffer[fileSize] = 0;
		LoadFromBuffer(buffer);

		free(buffer);

		return true;
	}
	virtual bool SaveToFile(const char* resourceName)
	{
		FILE* f = fopen(resourceName, "wb");
		if (!f)
			return false;

		CBuffer buf(0x10000); // 64KB
		RecursiveSaveToBuffer(buf, 0);

		fwrite(buf.Get(), buf.Tell(), 1, f);

		fclose(f);

		return true;
	}

	// load from text buffer
	virtual bool LoadFromBuffer(const char* pBuffer)
	{
		CBuffer buf;
		char token[100];
		bool got;

		buf.Write(pBuffer, strlen(pBuffer) + 1);
		buf.Seek(seek::set, 0);

		RemoveEverything();

		NewKeyValues* pPreviousKey = nullptr;
		NewKeyValues* pCurrentKey = this;

		while (1)
		{
			// get the key
			got = ReadToken(token, buf);

			// expected 'ident' but end of file
			if (!got)
				break;

			if (!pCurrentKey)
			{
				pCurrentKey = new NewKeyValues(token);

				if (pPreviousKey)
				{
					pPreviousKey->SetNextKey(pCurrentKey);
				}
			}
			else
			{
				// set name for this key
				pCurrentKey->SetName(token);
			}

			// get the value
			got = ReadToken(token, buf);

			// expected 'ident' or '{' but end of file
			if (!got)
				break;

			// get the key
			if (token[0] == '{')
			{
				pCurrentKey->RecursiveLoadFromBuffer(buf);
			}

			pPreviousKey = pCurrentKey;
			pCurrentKey = nullptr;
		}

		return true;
	}
	virtual bool SaveToBuffer(char* pBuffer, size_t* iSize)
	{
		CBuffer buf(0x10000);
		RecursiveSaveToBuffer(buf, 0);

		memcpy(pBuffer, buf.Get(), buf.Tell());

		*iSize = buf.Tell();

		return true;
	}

	// find a subkey
	virtual NewKeyValues* FindKey(const char* keyName, bool bCreate = false)
	{
		if (!keyName || !keyName[0])
			return this;

		NewKeyValues* lastItem = nullptr;
		NewKeyValues* dat;

		for (dat = m_pSub; dat != nullptr; dat = dat->m_pPeer)
		{
			lastItem = dat;

			if (!strcmp(keyName, dat->m_szName))
			{
				break;
			}
		}

		if (!dat)
		{
			if (bCreate)
			{
				dat = new NewKeyValues(keyName);

				if (lastItem)
				{
					lastItem->m_pPeer = dat;
				}
				else
				{
					m_pSub = dat;
				}
			}
			else
			{
				return nullptr;
			}
		}

		return dat;
	}

	// craete a subkey
	virtual NewKeyValues* CreateNewKey(void)
	{
		int newID = 1;

		for (NewKeyValues* dat = m_pSub; dat != nullptr; dat = dat->m_pPeer)
		{
			int val = atoi(dat->GetName());

			if (newID <= val)
			{
				newID = val + 1;
			}
		}

		char buf[12];
		sprintf(buf, "%d", newID);

		return CreateKey(buf);
	}
	virtual NewKeyValues* CreateKey(const char* keyName)
	{
		NewKeyValues* dat = new NewKeyValues(keyName);

		AddSubKey(dat);

		return dat;
	}

	// add/remove sub key
	virtual void AddSubKey(NewKeyValues* subKey)
	{
		if (m_pSub == nullptr)
		{
			m_pSub = subKey;
		}
		else
		{
			NewKeyValues* pTempDat = m_pSub;

			while (pTempDat->GetNextKey() != nullptr)
			{
				pTempDat = pTempDat->GetNextKey();
			}

			pTempDat->SetNextKey(subKey);
		}
	}
	virtual void RemoveSubKey(NewKeyValues* subKey)
	{
		if (!subKey)
			return;

		if (m_pSub == subKey)
		{
			m_pSub = m_pSub->m_pPeer;
		}
		else
		{
			NewKeyValues* dat = m_pSub;

			while (dat->m_pPeer)
			{
				if (dat->m_pPeer == subKey)
				{
					dat->m_pPeer = dat->m_pPeer->m_pPeer;
					break;
				}

				dat = dat->m_pPeer;
			}
		}

		subKey->m_pPeer = nullptr;
	}

	virtual NewKeyValues* GetNextKey(void)
	{
		return m_pPeer;
	}
	virtual void SetNextKey(NewKeyValues* dat)
	{
		m_pPeer = dat;
	}

	// get key
	virtual NewKeyValues* GetFirstSubKey(void)
	{
		NewKeyValues* dat = m_pSub;

		while (dat && !dat->m_pSub)
		{
			dat = dat->m_pPeer;
		}

		return dat;
	}
	virtual NewKeyValues* GetNextSubKey(void)
	{
		NewKeyValues* dat = m_pPeer;

		while (dat && !dat->m_pSub)
		{
			dat = dat->m_pPeer;
		}

		return dat;
	}
	virtual NewKeyValues* GetFirstValue(void)
	{
		NewKeyValues* dat = m_pSub;

		while (dat && dat->m_pSub)
		{
			dat = dat->m_pPeer;
		}

		return dat;
	}
	virtual NewKeyValues* GetNextValue(void)
	{
		NewKeyValues* dat = m_pPeer;

		while (dat && dat->m_pSub)
		{
			dat = dat->m_pPeer;
		}

		return dat;
	}

	// if not subkey return true
	virtual bool IsEmpty(const char* keyName = nullptr)
	{
		NewKeyValues* dat = FindKey(keyName);

		if (!dat)
			return true;

		if (!dat->m_pSub)
			return true;

		return false;
	}

	// set value
	virtual const char* GetString(const char* keyName = nullptr, const char* defaultValue = "")
	{
		NewKeyValues* dat = FindKey(keyName);

		if (dat && dat->m_szValue)
		{
			return dat->m_szValue;
		}

		return defaultValue;
	}
	virtual int GetInt(const char* keyName = nullptr, int defaultValue = 0)
	{
		NewKeyValues* dat = FindKey(keyName);

		if (dat)
		{
			return dat->m_iValue;
		}

		return defaultValue;
	}
	virtual float GetFloat(const char* keyName = nullptr, float defaultValue = 0)
	{
		NewKeyValues* dat = FindKey(keyName);

		if (dat)
		{
			return dat->m_flValue;
		}

		return defaultValue;
	}

	// get value
	virtual void SetString(const char* keyName, const char* value)
	{
		NewKeyValues* dat = FindKey(keyName, true);

		if (dat)
		{
			if (dat->m_szValue)
			{
				free(dat->m_szValue);
				dat->m_szValue = nullptr;
			}

			size_t len = strlen(value);
			dat->m_szValue = (char*)malloc(len + 1);
			strcpy(dat->m_szValue, value);
		}
	}
	virtual void SetInt(const char* keyName, int value)
	{
		NewKeyValues* dat = FindKey(keyName, true);

		if (dat)
		{
			if (dat->m_szValue)
			{
				free(dat->m_szValue);
				dat->m_szValue = nullptr;
			}

			dat->m_szValue = (char*)malloc(16);
			sprintf(dat->m_szValue, "%d", value);

			dat->m_iValue = value;
			dat->m_flValue = (float)value;
		}
	}
	virtual void SetFloat(const char* keyName, float value)
	{
		NewKeyValues* dat = FindKey(keyName, true);

		if (dat)
		{
			if (dat->m_szValue)
			{
				free(dat->m_szValue);
				dat->m_szValue = nullptr;
			}

			dat->m_szValue = (char*)malloc(16);
			sprintf(dat->m_szValue, "%.6f", value);

			dat->m_flValue = value;
			dat->m_iValue = (int)std::roundf(value);
		}
	}

	// remove all key/value
	virtual void Clear(void)
	{
		delete m_pSub;
		m_pSub = nullptr;
	}
	virtual void deleteThis(void)
	{
		delete this;
	}

private:
	void RemoveEverything(void)
	{
		NewKeyValues* dat;
		NewKeyValues* datNext = nullptr;

		for (dat = m_pSub; dat != nullptr; dat = datNext)
		{
			datNext = dat->m_pPeer;
			dat->m_pPeer = nullptr;
			delete dat;
		}

		for (dat = m_pPeer; dat && dat != this; dat = datNext)
		{
			datNext = dat->m_pPeer;
			dat->m_pPeer = nullptr;
			delete dat;
		}

		m_iValue = 0;
		m_flValue = 0;

		if (m_szValue)
		{
			free(m_szValue);
			m_szValue = nullptr;
		}
	}

	void RecursiveLoadFromBuffer(CBuffer& buf)
	{
		char token[2048];
		bool got;
		int type;

		while (1)
		{
			// get the key
			got = ReadToken(token, buf);

			// expected 'ident' or '}' but end of file
			if (!got)
				break;

			// close the key
			if (token[0] == '}')
				break;

			NewKeyValues* dat = CreateKey(token);

			// get the value
			got = ReadToken(token, buf);

			// expected '{' or 'ident' but end of file
			if (!got)
				break;

			// expected '{' or 'ident' but got '}'
			if (token[0] == '}')
				break;

			if (token[0] == '{')
			{
				dat->RecursiveLoadFromBuffer(buf);
			}
			else
			{
				type = UTIL_GetStringType(token);

				if (type == 1)
				{
					dat->m_iValue = std::atoi(token);
					dat->m_flValue = (float)dat->m_iValue;
				}
				else if (type == 2)
					dat->m_flValue = std::stof(token);

				if (dat->m_szValue)
				{
					free(dat->m_szValue);
					dat->m_szValue = nullptr;
				}

				size_t len = strlen(token);
				dat->m_szValue = (char*)malloc(len + 1);
				strcpy_s(dat->m_szValue, len + 1, token);
			}
		}
	}
	void RecursiveSaveToBuffer(CBuffer& buf, int indentLevel)
	{
		WriteIndents(buf, indentLevel);
		buf.Write("\"", 1);
		buf.Write(m_szName, strlen(m_szName));
		buf.Write("\"\n", 2);
		WriteIndents(buf, indentLevel);
		buf.Write("{\n", 2);

		for (NewKeyValues* dat = m_pSub; dat != nullptr; dat = dat->m_pPeer)
		{
			if (dat->m_pSub)
			{
				dat->RecursiveSaveToBuffer(buf, indentLevel + 1);
			}
			else
			{
				WriteIndents(buf, indentLevel + 1);
				buf.Write("\"", 1);
				buf.Write(dat->GetName(), strlen(dat->GetName()));
				buf.Write("\"\t\t\"", 4);
				buf.Write(dat->GetString(), strlen(dat->GetString()));
				buf.Write("\"\n", 2);
			}
		}

		WriteIndents(buf, indentLevel);
		buf.Write("}\n", 2);
	}

	void WriteIndents(CBuffer& buf, int indentLevel)
	{
		for (int i = 0; i < indentLevel; ++i)
		{
			buf.Write("\t", 1);
		}
	}

	void Init(void)
	{
		if (m_szName)
		{
			free(m_szName);
			m_szName = nullptr;
		}

		if (m_szValue)
		{
			free(m_szValue);
			m_szValue = nullptr;
		}

		m_iValue = 0;
		m_flValue = 0;

		m_pPeer = nullptr;
		m_pSub = nullptr;
	}
	bool ReadToken(char* token, CBuffer& buf)
	{
		char* pw = token;
		char ch;

		while (1)
		{
		skipwhite:
			do { ch = buf.getc(); } while (ch > 0 && ch < 0x80 && isspace(ch));	// UTF-8

			if (ch == '/')
			{
				ch = buf.getc();

				if (ch == '/')
				{
					do { ch = buf.getc(); } while (ch != '\n');

					goto skipwhite;
				}
			}

			if (ch == '{' || ch == '}')
			{
				pw[0] = ch;
				pw[1] = 0;

				return true;
			}

			if (ch == '"')
			{
				do { *pw = buf.getc(); } while (*(pw++) != '"');

				*(--pw) = 0;

				return true;
			}

			if (ch == '\0')
			{
				*pw = 0;

				return false;
			}
		}
	}

	char* m_szName;

	char* m_szValue;
	int m_iValue;
	float m_flValue;

	NewKeyValues* m_pPeer;
	NewKeyValues* m_pSub;
};

module;

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS	// This is a very unsafe module.
#endif

// C++
#include <concepts>	// std::integral, etc...
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

// Entry - Subkey or KeyValue
// Subkey - Contains only Entries, but no value for itself.
// KeyValue - A pair of string.
// Key - the "Name" of a Subkey.
// Value - the "Number/String" part of a KeyValue.

export struct ValveKeyValues
{
	ValveKeyValues(const char* pszName) noexcept
	{
		Init();
		SetName(pszName);
	}
	//ValveKeyValues(const std::integral auto& index) noexcept
	//{
	//	Init();
	//	SetName(std::to_string(index).c_str());
	//}
	virtual ~ValveKeyValues(void) noexcept
	{
		RemoveEverything();
	}

	// set/get name
	const char* GetName(void) const noexcept
	{
		if (m_pszName)
			return m_pszName;

		return "";
	}
	void SetName(const char* pszName) noexcept
	{
		if (!pszName)
			pszName = "";

		if (m_pszName)
		{
			free(m_pszName);
			m_pszName = nullptr;
		}

		size_t len = strlen(pszName);
		m_pszName = (char*)malloc(len + 1);
		strcpy(m_pszName, pszName);
	}

	// load/save file
	bool LoadFromFile(const char* resourceName) noexcept
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
	bool SaveToFile(const char* resourceName) noexcept
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
	bool LoadFromBuffer(const char* pBuffer) noexcept
	{
		CBuffer buf;
		char token[100];
		bool got;

		buf.Write(pBuffer, strlen(pBuffer) + 1);
		buf.Seek(seek::set, 0);

		RemoveEverything();

		ValveKeyValues* pPreviousKey = nullptr;
		ValveKeyValues* pCurrentKey = this;

		while (true)
		{
			// get the key
			got = ReadToken(token, buf);

			// expected 'ident' but end of file
			if (!got)
				break;

			if (!pCurrentKey)
			{
				pCurrentKey = new ValveKeyValues(token);

				if (pPreviousKey)
				{
					pPreviousKey->m_pPeer = pCurrentKey;
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
	bool SaveToBuffer(char* pBuffer, size_t* piSize) noexcept
	{
		CBuffer buf(0x10000);
		RecursiveSaveToBuffer(buf, 0);

		memcpy(pBuffer, buf.Get(), buf.Tell());

		*piSize = buf.Tell();

		return true;
	}

	// find a entry
	ValveKeyValues* FindEntry(const char* pszName, bool bCreate = false) noexcept
	{
		if (!pszName || !pszName[0])
			return this;

		ValveKeyValues* lastItem = nullptr;
		ValveKeyValues* dat;

		for (dat = m_pSub; dat != nullptr; dat = dat->m_pPeer)
		{
			lastItem = dat;

			if (!strcmp(pszName, dat->m_pszName))
			{
				break;
			}
		}

		if (!dat)
		{
			if (bCreate)
			{
				dat = new ValveKeyValues(pszName);

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

	// craete a entry
	ValveKeyValues* CreateEntry(const char* pszName = nullptr) noexcept	// nullptr on pszName to represent a auto-index entry name.
	{
		ValveKeyValues* dat = nullptr;

		if (!pszName)
		{
			int index = 1;

			for (ValveKeyValues* dat = m_pSub; dat != nullptr; dat = dat->m_pPeer)
			{
				if (auto val = atoi(dat->GetName()); index <= val)
					index = val + 1;
			}

			dat = new ValveKeyValues(std::to_string(index).c_str());
		}
		else
			dat = new ValveKeyValues(pszName);

		AddEntry(dat);
		return dat;
	}

	// add/remove an existing entry
	void AddEntry(ValveKeyValues* pEntry) noexcept
	{
		if (m_pSub == nullptr)
		{
			m_pSub = pEntry;
		}
		else
		{
			ValveKeyValues* pTempDat = m_pSub;

			while (pTempDat->GetNextEntry() != nullptr)
			{
				pTempDat = pTempDat->GetNextEntry();
			}

			pTempDat->m_pPeer = pEntry;
		}
	}
	void RemoveEntry(ValveKeyValues* pEntry) noexcept
	{
		if (!pEntry)
			return;

		if (m_pSub == pEntry)
		{
			m_pSub = m_pSub->m_pPeer;
		}
		else
		{
			ValveKeyValues* dat = m_pSub;

			while (dat->m_pPeer)
			{
				if (dat->m_pPeer == pEntry)
				{
					dat->m_pPeer = dat->m_pPeer->m_pPeer;
					break;
				}

				dat = dat->m_pPeer;
			}
		}

		pEntry->m_pPeer = nullptr;
	}

	// get entry
	ValveKeyValues* GetFirstEntry(void) const noexcept
	{
		return m_pSub;
	}
	ValveKeyValues* GetNextEntry(void) const noexcept
	{
		return m_pPeer;
	}
	ValveKeyValues* GetFirstSubkey(void) const noexcept
	{
		ValveKeyValues* dat = m_pSub;

		while (dat && !dat->m_pSub)
		{
			dat = dat->m_pPeer;
		}

		return dat;
	}
	ValveKeyValues* GetNextSubkey(void) const noexcept
	{
		ValveKeyValues* dat = m_pPeer;

		while (dat && !dat->m_pSub)
		{
			dat = dat->m_pPeer;
		}

		return dat;
	}
	ValveKeyValues* GetFirstKeyValue(void) const noexcept
	{
		ValveKeyValues* dat = m_pSub;

		while (dat && dat->m_pSub)
		{
			dat = dat->m_pPeer;
		}

		return dat;
	}
	ValveKeyValues* GetNextKeyValue(void) const noexcept
	{
		ValveKeyValues* dat = m_pPeer;

		while (dat && dat->m_pSub)
		{
			dat = dat->m_pPeer;
		}

		return dat;
	}

	// return true if not subkey or value included.
	bool IsEmpty(void) const noexcept
	{
		return !m_pSub && !m_pszValue;
	}
	bool IsKeyValue(void) const noexcept
	{
		return m_pszValue != nullptr;
	}
	bool IsSubkey(void) const noexcept
	{
		return m_pSub != nullptr;
	}

	// value
	template<typename T> T GetValue(const char* pszSubkeyName = nullptr) noexcept requires(std::integral<T> || std::floating_point<T> || std::convertible_to<T, std::string>)
	{
		ValveKeyValues* dat = FindEntry(pszSubkeyName);
		if (!dat || !dat->m_pszValue)
		{
			if constexpr (std::convertible_to<T, std::string>)
				return "";
			else
				return 0;
		}

		if constexpr (std::floating_point<T>)
		{
			return static_cast<T>(dat->m_flValue);
		}
		else if constexpr (std::integral<T>)
		{
			return static_cast<T>(std::roundf(dat->m_flValue));
		}
		else
		{
			return dat->m_pszValue;
		}
	}
	template<typename T> bool SetValue(const char* pszSubkeyName, const T& Value) noexcept requires(std::integral<T> || std::floating_point<T> || std::convertible_to<T, std::string>)	// Create new entry on no found.
	{
		ValveKeyValues* dat = FindEntry(pszSubkeyName, true);
		if (!dat)
			return false;

		if (dat->m_pszValue)
		{
			free(dat->m_pszValue);
			dat->m_pszValue = nullptr;
		}

		if constexpr (std::integral<T> || std::floating_point<T>)
		{
			std::string szValue = std::to_string(Value);
			dat->m_pszValue = (char*)malloc(szValue.length() + 1);
			strcpy_s(dat->m_pszValue, szValue.length() + 1, szValue.c_str());

			dat->m_flValue = static_cast<double>(Value);
		}
		else	// i.e. strings.
		{
			if constexpr (std::is_same_v<std::decay_t<T>, char*>)
			{
				size_t len = strlen(Value);
				dat->m_pszValue = (char*)malloc(len + 1);
				strcpy(dat->m_pszValue, Value);
			}
			else	// i.e. std::string.
			{
				dat->m_pszValue = (char*)malloc(Value.length() + 1);
				strcpy_s(dat->m_pszValue, Value.length() + 1, Value.c_str());
			}

			switch (UTIL_GetStringType(dat->m_pszValue))
			{
			default:
			case 0:	// String
				dat->m_flValue = 0;
				break;

			case 1:	// Integer
			case 2:	// Floating point
				dat->m_flValue = std::atof(dat->m_pszValue);
				break;
			}
		}

		return true;
	}

	// remove all key/value
	virtual void Clear(void) noexcept
	{
		delete m_pSub;
		m_pSub = nullptr;
	}
	virtual void deleteThis(void) noexcept
	{
		delete this;
	}

private:
	void RemoveEverything(void) noexcept
	{
		ValveKeyValues* dat;
		ValveKeyValues* datNext = nullptr;

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

		m_flValue = 0;

		if (m_pszValue)
		{
			free(m_pszValue);
			m_pszValue = nullptr;
		}
	}

	void RecursiveLoadFromBuffer(CBuffer& buf) noexcept
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

			ValveKeyValues* dat = CreateEntry(token);

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

				if (type == 1 || type == 2)
					dat->m_flValue = std::stof(token);

				if (dat->m_pszValue)
				{
					free(dat->m_pszValue);
					dat->m_pszValue = nullptr;
				}

				size_t len = strlen(token);
				dat->m_pszValue = (char*)malloc(len + 1);
				strcpy_s(dat->m_pszValue, len + 1, token);
			}
		}
	}
	void RecursiveSaveToBuffer(CBuffer& buf, int indentLevel) noexcept
	{
		WriteIndents(buf, indentLevel);
		buf.Write("\"", 1);
		buf.Write(m_pszName, strlen(m_pszName));
		buf.Write("\"\n", 2);
		WriteIndents(buf, indentLevel);
		buf.Write("{\n", 2);

		for (ValveKeyValues* dat = m_pSub; dat != nullptr; dat = dat->m_pPeer)
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
				buf.Write(dat->GetValue<const char*>(), strlen(dat->GetValue<const char*>()));
				buf.Write("\"\n", 2);
			}
		}

		WriteIndents(buf, indentLevel);
		buf.Write("}\n", 2);
	}

	void WriteIndents(CBuffer& buf, int indentLevel) noexcept
	{
		for (int i = 0; i < indentLevel; ++i)
		{
			buf.Write("\t", 1);
		}
	}

	void Init(void) noexcept
	{
		if (m_pszName)
		{
			free(m_pszName);
			m_pszName = nullptr;
		}

		if (m_pszValue)
		{
			free(m_pszValue);
			m_pszValue = nullptr;
		}

		m_flValue = 0;

		m_pPeer = nullptr;
		m_pSub = nullptr;
	}
	bool ReadToken(char* token, CBuffer& buf) noexcept
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

	char* m_pszName{ nullptr };

	char* m_pszValue{ nullptr };
	double m_flValue{ 0.0 };

	ValveKeyValues* m_pPeer{ nullptr };
	ValveKeyValues* m_pSub{ nullptr };
};

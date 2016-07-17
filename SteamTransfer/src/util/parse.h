#ifndef UTIL_PARSE_H_
#define UTIL_PARSE_H_

#include <string>
#include <unordered_map>

/********************************************
Simple Valve Data Format (VDF) parser
NIH, but avoids licensing issues/bloat from pulling in the Source SDK

Limitations:
* Cannot parse any of the #-macros such as #base and #include
* Assumes all keys are strings and all values are strings or objects
********************************************/

struct VDFObject;

class VDFParser
{
public:
	bool parse(const std::wstring& vdfText, VDFObject& root);
};

struct VDFObject
{
	std::wstring rawValue = L"";

	VDFObject& operator[](const wchar_t* key)
	{
		if(rawValue.find_first_of('{') != std::string::npos)
		{
			if(!_cache.size())
			{
				_parse();
			}

			return _cache[key];
		}
		else
			return *this;
	}

	VDFObject& operator=(const wchar_t* value)
	{
		rawValue = value;
		return *this;
	}

	VDFObject& operator=(const std::wstring& value)
	{
		rawValue.assign(value);
		return *this;
	}

	operator std::wstring() const
	{
		return rawValue;
	}

	const std::unordered_map<std::wstring, VDFObject>& getAll()
	{
		if (!_cache.size())
			_parse();

		return _cache;
	}

private:

	void _parse();

	std::unordered_map<std::wstring, VDFObject> _cache;
};


#endif
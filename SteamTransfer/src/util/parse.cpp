#include "parse.h"
#include <string>
#include <algorithm>

size_t find_matching_closing_brace(const std::wstring& str)
{
	size_t openCount = 0;
	size_t closeCount = 0;

	size_t position = 0;

	while(str[position] != '\0')
	{
		if(str[position] == '}')
		{
			if(openCount == closeCount)
				break;
			else
				++closeCount;
		}
		else if(str[position] == '{')
			++openCount;

		++position;
	}

	return position;
}

bool VDFParser::parse(const std::wstring& vdfText, VDFObject& root)
{
	//Pre-parse check - perform some basic sanity checks on the data
	//All open brackets must close, all strings must eventually end.
	if (!(
		std::count(vdfText.begin(), vdfText.end(), '{') == std::count(vdfText.begin(), vdfText.end(), '}')
		&& std::count(vdfText.begin(), vdfText.end(), '"') % 2 == 0
		))
	{
		return false;
	}

	root = vdfText;

	return true;
}

void VDFObject::_parse()
{
	//populate cache.
	// don't parse objects as values, just store their text for lazy parsing later.

	const size_t maxLength = rawValue.length();

	size_t i = 0;
	size_t next;

	bool isKey = true;
	std::pair<std::wstring, VDFObject> pair;

	do
	{
		i = rawValue.find_first_of(L"\"{}\0", i);

		if(i == std::string::npos)
			return; //something definitely went wrong.

		switch(rawValue[i])
		{
		case '\"': //parse as a key OR a value
			next = rawValue.find_first_of(L"\"", i + 1);

			if(next != std::string::npos)
			{
				if(isKey)
					pair.first = rawValue.substr(i + 1, next - i - 1);
				else
				{
					//Special case: the VDF already escape-slashes paths for us
					//But this makes the windows shell APIs whine.
					//So, remove the extra escape slashes, but only for paths
					std::wstring cleaned = rawValue.substr(i + 1, next - i - 1);

					size_t pos = cleaned.find(L"\\\\");
					while (pos != std::string::npos)
					{
						cleaned.erase(pos, 1);
						pos = cleaned.find(L"\\\\");
					}

					pair.second = cleaned;
					_cache.insert(pair);
				}

				isKey = !isKey;

				i = next;
			}

			break;

		case '{': //Found an object, parse as a value
		{
			//The first character being { is expected behaviour of subobjects. Ignore.
			if (i == 0)
				continue;

			isKey = false;
			size_t pos = find_matching_closing_brace(&rawValue[i + 1]);
			pair.second = rawValue.substr(i, i + pos);
			i = pos;

			_cache.insert(pair);
		}
			break;

		case '}': //Found an object ending - we should end this fragparse here too.
		case '\0': //EOF
		default: //Huh?
			return;
			break;
		}

	} while(++i < maxLength);
}
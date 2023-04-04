#pragma once

#include <iostream>
#include <algorithm>

class CBuffType
{
private:
	bool _cand;

public:
	CBuffType(bool wall = true)
		: size(512), actualRead(0), data(new char[size]), _cand(true), writeAll(wall)
	{ }

	CBuffType(size_t s, bool wall = true) : size(s), actualRead(0),
		data(new char[size]), _cand(true), writeAll(wall)
	{ }

	CBuffType(char* d, size_t s = 512, bool wall = true) : size(s), actualRead(0),
		data(d), _cand(false), writeAll(wall)
	{ }

	CBuffType(const CBuffType& other, bool wall = true) : size(other.size), actualRead(0),
		data(new char[other.size]), _cand(true), writeAll(wall)
	{
		std::copy(other.data, other.data + size, data);
	}

	~CBuffType() { if (_cand) delete[] data; }

	size_t size;
	size_t actualRead;
	char* data;
	bool writeAll;

	void init() { std::fill_n(data, size, 0); }
};

std::ostream& operator<<(std::ostream& _ofs, const CBuffType& bt)
{
	size_t nToWrite = bt.writeAll ? bt.size : bt.actualRead;

	if (nToWrite && bt.size <= nToWrite) //Can't write more than actual size
		_ofs.write(bt.data, nToWrite);
	return _ofs;
}

std::istream& operator>>(std::istream& _ifs, CBuffType& bt)
{
	size_t nRead = 0, toRead, nCurr;

	toRead = bt.size;
	do
	{
		_ifs.read(bt.data + nRead, toRead);
		nCurr = _ifs.gcount();
		nRead += nCurr;
		if (!nCurr || _ifs.eof()) break; //EOF?
		toRead -= nRead;
	} while (nRead < bt.size);
	bt.actualRead = nRead;
	if (nRead < bt.size && bt.writeAll) //Pad remaining data with zeros when needed
		std::fill_n(bt.data + nRead, bt.size - nRead, 0);
	return _ifs;
}

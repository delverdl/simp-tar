#pragma once

#include <cstdio>
#include <exception>
#include <fstream>
#include <string>

#include "CBuffType.hpp"

class CTarArchive
{

public:

	CTarArchive()
		: _failed(false), _tempFile(temporaryFile()), _move(false)
	{ }

	CTarArchive(const std::string& fileName)
		: _failed(true), _name(fileName), _tempFile(temporaryFile()), _move(false)
	{
		open();
	}

	~CTarArchive()
	{
		try
		{
			close();
		}
		catch (std::exception& ex)
		{
			std::cerr << "Error closing TAR file: " << ex.what() << std::endl;
		}
		if (_failed && !_name.empty() && _name[0] != ' ')
			::remove(_name.c_str());
	}

	void setFileName(const std::string& sn) { _name = sn; }

	std::string name() const { return _name; }

	void setMove(bool mov) { _move = mov; }

	bool move() const { return _move; }

	void open()
	{
		if (_ofs.is_open()) return;
		_ofs.exceptions(std::ios_base::badbit | std::ios_base::failbit);
		_ofs.open(_name, std::ios_base::binary | std::ios_base::trunc);
	}

	void close()
	{
		if (_ofs.is_open())
		{
			finalizeEntry();
			addPadding();
			_ofs.close();
			_failed = false;
		}
	}

	void addFile(const std::string& name, time_t t = 0)
	{   //Add file from buffered
		if (!_ofs.is_open()) return;
		//End previous entry
		finalizeEntry();
		//New file information
		_newTime = t;
		_newName = name;
		//Use temporary file for user to add data
		_otemp.exceptions(std::ios_base::badbit | std::ios_base::failbit);
		_otemp.open(_tempFile, std::ios_base::binary | std::ios_base::trunc);
	}

	void addData(const std::string& data)
	{
		if (!_otemp.is_open()) return;
		_otemp << data;
	}

	void addFileFromPath(const std::string& path)
	{
		STarHeader th;
		struct stat fst;
		size_t idx;

		idx = path.find_last_of('\\');
		if (idx == std::string::npos) idx = path.find_last_of('/');
		idx++;
		std::copy(path.c_str() + idx, path.c_str() + 
			path.length() + 1 /*also copy zero at end*/, th.name);
		stat(path.c_str(), &fst);
		th.setSize((size_t) fst.st_size);
		th.setTime((time_t) fst.st_mtime);
		th.refreshCrc(); //Update HEADER CRC
		addFileFromPath(path, th);
	}

private:

	struct STarHeader
	{
		char name[100];			//0x0000	<NAME>
		char mode[8];			//0x0064	"000644 " (octal file mode)
		char uid[8];			//0x006C	0x00 (8)
		char gid[8];			//0x0074	0x00 (8)
		char size[12];			//0x007C	"00000000011" (octal text)
		char mtime[12];			//0x0088	"14410345740" (time since epoch, octal)
		char chksum[8];			//0x0094	"0005511" (octal sum)
		char typeflag;			//0x009C	0x30 (File)
		char linkname[100];		//0x009D	0x00 (100)
		char magic[6];			//0x0101	0x75 0x73 0x74 0x61 0x72 0x20 (ustar )
		char version[2];		//0x0107	0x20 0x00
		char uname[32];			//0x0109	0x00 (32)
		char gname[32];			//0x0129	0x00 (32)
		char devmajor[8];		//0x0149	0x00 (8)
		char devminor[8];		//0x0151	0x00 (8)
		char prefix[155];		//0x0159	0x00 (155)
		char padding[12];		//0x01F4	0x00 (12)
								//...0x200

		STarHeader()
		{
			time_t tmt;

			//Avoid constructor warning
			uid[0]=gid[0]=size[0]=mtime[0]=chksum[0]=typeflag=linkname[0]=
				magic[0]=version[0]=uname[0]=gname[0]=devmajor[0]=devminor[0]=
				prefix[0]=padding[0]=name[0]=mode[0]=0;
			//Zero all memory area
			std::fill_n((char*)this, 512, 0);
			memcpy_s(mode, 8, "000644 ", 8);
			memcpy_s(size, 12, "00000000000", 12);
			typeflag = 0x30;
			sprintf_s(mtime, "%011I64o", time(&tmt));
			memcpy_s(magic, 6, "ustar ", 6);
			version[0] = 0x20;
		}

		static int crc(const STarHeader& th)
		{
			int i, sum = 0;

			for (i = 0; i < 512; i++)
				sum += ((unsigned char*)(&th))[i];
			for (i = 0; i < 8; i++)
				sum += (' ' - (unsigned char)th.chksum[i]);
			return sum;
		}

		void refreshCrc()
		{
			int c = crc(*this);

			sprintf_s(chksum, "%07o", c);
		}

		void setSize(size_t ns)
		{
			sprintf_s(size, "%011I64o", ns);
		}

		void setTime(time_t tt)
		{
			sprintf_s(mtime, "%011I64o", tt);
		}

	};

	bool _failed;
	std::string _name;
	std::ofstream _ofs;
	std::ofstream _otemp;
	std::string _tempFile;
	std::string _newName;
	time_t _newTime;
	bool _move;

	void addPadding()
	{
		CBuffType _buff;

		_buff.init();
		_ofs << _buff << _buff;
	}

	void addFileFromPath(const std::string& path, const STarHeader& th)
	{
		_ofs << CBuffType((char*) &th);

		CBuffType bt;
		std::ifstream ifs(path, std::ios_base::binary);

		if (!ifs.good())
			throw new std::exception("Can't open input file!");
		while (!ifs.eof())
		{
			ifs >> bt;
			if (ifs.fail() && !ifs.eof())
				throw new std::exception("Can't read input file!");
			_ofs << bt;
		}
		if (_move)
		{
			ifs.close();
			::remove(path.c_str());
		}
	}

	void finalizeEntry()
	{
		if (_otemp.is_open())
		{
			size_t fss = _otemp.tellp();
			STarHeader th;

			_otemp.close(); //Close temporary file

			std::copy(_newName.c_str(), _newName.c_str() +
				_newName.length() + 1 /*also copy zero at end*/, th.name);
			
			th.setSize(fss);
			th.setTime(_newTime);
			th.refreshCrc(); //Update HEADER CRC
			addFileFromPath(_tempFile, th);

			_newName.clear();
		}
	}

	template <class T>
	void release(T*& t)
	{
		delete t;
		t = nullptr;
	}

	std::string temporaryFile()
	{
		size_t rSize = 0;
		char _bp[265];
		std::string sOut = "tarc_0000.tmp";

		if (!getenv_s(&rSize, _bp, "TEMP"))
		{
			_bp[rSize] = 0;
			sOut = std::string(_bp) + std::string("\\") + sOut;
		}
		return sOut;
	}

};
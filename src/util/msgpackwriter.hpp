#pragma once

#include <fstream>
#include <string>

#include <msgpack.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

template<typename T>
class MsgpackWriter
{
	std::ofstream m_fo;
	boost::iostreams::filtering_ostream m_so;
	msgpack::packer<std::ostream> m_pac;

	MsgpackWriter(const MsgpackWriter&) = delete;
	MsgpackWriter& operator=(const MsgpackWriter&) = delete;

public:
	MsgpackWriter(std::string filename)
	: m_fo(filename, std::ios_base::binary)
	, m_so()
	, m_pac(&m_so)
	{
		m_so.push(boost::iostreams::gzip_compressor());
		m_so.push(m_fo);
	}

	~MsgpackWriter()
	{
		// Closing the buffers in order is very important (UB)
		boost::iostreams::close(m_so);
		m_fo.close();
	}

	void write(const T& x)
	{
		m_pac.pack(x);
	}
};

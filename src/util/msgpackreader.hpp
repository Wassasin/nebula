#pragma once

#include <fstream>
#include <string>
#include <memory>

#include <msgpack.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/operations.hpp>

template<typename T>
class MsgpackReader
{
	std::ifstream m_fi;
	boost::iostreams::filtering_istream m_si;
	msgpack::unpacker m_pac;
	std::shared_ptr<msgpack::zone> m_zone;

	MsgpackReader(const MsgpackReader&) = delete;
	MsgpackReader& operator=(const MsgpackReader&) = delete;

	bool consume()
	{
		m_pac.reserve_buffer(32*1024);

		auto len = boost::iostreams::read(m_si, m_pac.buffer(), m_pac.buffer_capacity());

		if(len <= 0)
			return false;

		m_pac.buffer_consumed(len);

		return true;
	}

public:

	MsgpackReader(std::string filename)
	: m_fi(filename, std::ios_base::binary)
	, m_si()
	, m_pac(64*1024*1024) // We use large objects, which will fail to load if buffer is not big enough
	, m_zone()
	{
		m_si.push(boost::iostreams::gzip_decompressor());
		m_si.push(m_fi);
	}

	~MsgpackReader()
	{
		boost::iostreams::close(m_si);
		m_fi.close();
	}

	bool read(T& x)
	{
		m_zone.reset(); // Destruct and release memory of previous value
		msgpack::unpacked result;
		if(!m_pac.next(&result))
		{
			if(!consume())
				return false;

			if(!m_pac.next(&result))
				std::logic_error("Msgpack buffer should contain something now.");
		}

		msgpack::object obj = result.get();

		try
		{
			obj.convert(&x); // Reference will live until next read call (see msgpack::zone, m_zone)
		} catch(msgpack::type_error e)
		{
			throw std::runtime_error("Msgpack type problem. Might be a broken file; or that your object was too big.");
		}

		auto tmp = result.zone();
		m_zone.reset(tmp.release()); // Yield pointer to shared_ptr, release ownership

		if(m_pac.message_size() > 64*1024*1024)
			throw std::runtime_error("Msgpack message is too large");

		return true;
	}
};

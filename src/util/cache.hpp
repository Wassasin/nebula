#pragma once

#include <functional>
#include <boost/filesystem.hpp>

#include "msgpackreader.hpp"
#include "msgpackwriter.hpp"

template<typename T>
class cache
{
private:
	cache() = delete;
	cache(cache&) = delete;
	cache& operator=(cache&) = delete;

public:
	static T acquire(const std::string& filename, std::function<T()> generate_callback)
	{
		if(boost::filesystem::exists(filename))
		{
			MsgpackReader<T> reader(filename);
			T result;
			reader.read(result);
			return result;
		}
		else
		{
			T result = generate_callback();
			MsgpackWriter<T> writer(filename);
			writer.write(result);
			return result;
		}
	}
};

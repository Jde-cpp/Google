#include <nlohmann/json.hpp>

#pragma warning( disable : 4245) 
#include <boost/crc.hpp> 
#pragma warning( default : 4245) 
#include <boost/noncopyable.hpp>
#include <boost/system/error_code.hpp>
#ifndef __INTELLISENSE__
	#include <spdlog/spdlog.h>
	#include <spdlog/sinks/basic_file_sink.h>
	#include <spdlog/fmt/ostr.h>
#endif

#include "DateTime.h"
#include "Exception.h"
#include "JdeAssert.h"
#include "TypeDefs.h"
#include "log/Logging.h"
#include "Settings.h"
#include "collections/UnorderedMap.h"
#include "io/File.h"
#include "../../Ssl/source/Ssl.h"


namespace Jde
{
	using nlohmann::json;
	using Collections::UnorderedMap;
}


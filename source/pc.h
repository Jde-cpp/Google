#include <jde/TypeDefs.h>
DISABLE_WARNINGS

#include <boost/crc.hpp> 

#include <boost/noncopyable.hpp>
#include <boost/system/error_code.hpp>
#ifndef __INTELLISENSE__
	#include <spdlog/spdlog.h>
	#include <spdlog/sinks/basic_file_sink.h>
	#include <spdlog/fmt/ostr.h>
#endif

ENABLE_WARNINGS 
#include "../../Framework/source/DateTime.h"
//#include <jde/Exception.h>
#include <jde/Assert.h>

#include <jde/Log.h>
//#include "../../Framework/source/Settings.h"
#include "../../Framework/source/collections/UnorderedMap.h"
//#include <jde/io/File.h>
//#include "../../Ssl/source/Ssl.h"


namespace Jde
{
	//using nlohmann::json;
	using Collections::UnorderedMap;
}


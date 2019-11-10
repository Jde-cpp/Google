#pragma once

#ifdef JDE_GOOGLE_EXPORTS
	#ifdef _MSC_VER
		#define JDE_GOOGLE_EXPORT __declspec( dllexport )
	#else
		#define JDE_GOOGLE_EXPORT __attribute__((visibility("default")))
	#endif
#else 
	#ifdef _MSC_VER
		#define JDE_GOOGLE_EXPORT __declspec( dllimport )
		#if NDEBUG
			#pragma comment(lib, "Jde.IO.Drive.Google.lib")
		#else
			#pragma comment(lib, "Jde.IO.Drive.Google.lib")
		#endif
	#else
		#define JDE_GOOGLE_EXPORT
	#endif
#endif
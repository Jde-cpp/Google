#include "DriveTypes.h"
#include "../../Framework/source/io/DiskWatcher.h"

namespace Jde::IO::Drive::Google
{
	File::File()noexcept
	{}

	File::File( const File& file, path path)noexcept:
		File{ file }
	{
		Path = path;
	}

/*	File::File( const File& file )//:
		// Id{ file.Id },
		// CreatedTime{file.CreatedTime},
		// Kind{file.Kind},
		// MimeType{file.MimeType},
		// ModifiedTime{file.ModifiedTime},
		// Name{file.Name},
		// Parents{file.Parents},
		// Path{file.Path},
		// Size{file.Size}
	{
		DBG0( "File::File" );
		Id = file.Id;
		CreatedTime=file.CreatedTime;
		Kind=file.Kind;
		MimeType=file.MimeType;
		ModifiedTime=file.ModifiedTime;
		Name=file.Name;
		Parents=file.Parents;
		Path=file.Path;
		Size=file.Size;
	}
	*/
	File::File( const IDirEntry& entry, string_view parentId )noexcept:
		Id{""},
		CreatedTime{ entry.CreatedTime },
		ModifiedTime{ entry.CreatedTime },
		Name{ entry.Path.stem().string() }
	{
		Parents.push_back( string{parentId} );
	}
}
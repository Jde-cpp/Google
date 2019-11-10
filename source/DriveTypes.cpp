#include "stdafx.h"
#include "DriveTypes.h"
#include "../../Framework/source/io/DiskWatcher.h"

namespace Jde::IO::Drive::Google
{
	File::File( const File& file, const fs::path& path):
		File(file)
	{
		Path = path;
	}
	File::File( const IDirEntry& entry, string_view parentId ):
		Id{""},
		CreatedTime{ entry.CreatedTime },
		ModifiedTime{ entry.CreatedTime },
		Name{ entry.Path.stem().string() },
		Parents{ string{parentId} }
	{}
}
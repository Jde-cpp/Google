#pragma once
#include "./Exports.h"
#include "../../Framework/source/io/DiskWatcher.h"
//#include "io/drive/DriveApi.h"

extern "C" JDE_GOOGLE_EXPORT Jde::IO::IDrive* LoadDrive();

namespace Jde::IO::Drive
{
	struct GoogleDrive final:	public IDrive
	{
		GoogleDrive(){ DBG0("GoogleDrive::GoogleDrive"); };
		~GoogleDrive(){ DBG0("GoogleDrive::~GoogleDrive"); }
		map<string,IDirEntryPtr> Recursive( const fs::path& dir )noexcept(false) override;
		IDirEntryPtr Get( const fs::path& path )noexcept(false){ THROW( Exception("Not Implemented") ); }
		IDirEntryPtr Save( const fs::path& path, const vector<char>& bytes, const IDirEntry& dirEntry )noexcept(false) override{ return Save( path, bytes, dirEntry, 0 ); }
		IDirEntryPtr Save( const fs::path& path, const vector<char>& bytes, const IDirEntry& dirEntry, uint retry )noexcept(false);
		IDirEntryPtr CreateFolder( const fs::path& path, const IDirEntry& dirEntry )noexcept(false) override;
		//VectorPtr<char> Load( const fs::path& path )noexcept(false) override;
		VectorPtr<char> Load( const IDirEntry& dirEntry )noexcept(false) override;
		void Remove( const fs::path& path )noexcept(false) override;
		void Trash( const fs::path& path )noexcept(false) override;
	};
}

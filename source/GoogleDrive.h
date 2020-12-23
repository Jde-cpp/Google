#pragma once
#include "./Exports.h"
#include "../../Framework/source/io/DiskWatcher.h"
//#include "io/drive/DriveApi.h"

extern "C" JDE_GOOGLE_EXPORT Jde::IO::IDrive* LoadDrive();

namespace Jde::IO::Drive
{
	struct GoogleDrive final:	public IDrive
	{
		GoogleDrive(){ DBG0("GoogleDrive::GoogleDrive"sv); };
		~GoogleDrive(){ DBG0("GoogleDrive::~GoogleDrive"sv); }
		map<string,IDirEntryPtr> Recursive( path dir )noexcept(false) override;
		IDirEntryPtr Get( path path )noexcept(false) override{ THROW( Exception("Not Implemented") ); }
		void SoftLink( path existingFile, path newSymLink )noexcept(false) override{ THROW( Exception("Not Implemented") ); }
		IDirEntryPtr Save( path path, const vector<char>& bytes, const IDirEntry& dirEntry )noexcept(false) override{ return Save( path, bytes, dirEntry, 0 ); }
		IDirEntryPtr Save( path path, const vector<char>& bytes, const IDirEntry& dirEntry, uint retry )noexcept(false);
		IDirEntryPtr CreateFolder( path path, const IDirEntry& dirEntry )noexcept(false) override;
		//VectorPtr<char> Load( path path )noexcept(false) override;
		VectorPtr<char> Load( const IDirEntry& dirEntry )noexcept(false) override;
		void Remove( path path )noexcept(false) override;
		void Trash( path path )noexcept(false) override;
		void TrashDisposal( TimePoint latestDate )noexcept(false) override;
		void Restore( sv name )noexcept(false) override;
	};
}

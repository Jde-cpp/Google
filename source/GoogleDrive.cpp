#include "stdafx.h"
#include "GoogleDrive.h"
#include "DriveTypes.h"
#include "GoogleApi.h"

#define var const auto

Jde::IO::IDrive* LoadDrive()
{
	return new Jde::IO::Drive::GoogleDrive();
}

namespace Jde::IO::Drive
{
	using Jde::Google::AccessToken;
	using Jde::IO::Drive::Google::FileList;
	using Jde::IO::Drive::Google::FilePtr;
	struct DirEntry : IDirEntry
	{
		DirEntry( const Google::File& file ):
			IDirEntry{ file.IsDirectory() ? EFileFlags::Directory : EFileFlags::None, file.Path, file.Size, file.CreatedTime, file.ModifiedTime },
			File{ file }
		{}
		//bool IsDirectory()const noexcept override{ return File.IsDirectory(); }
		Google::File File;
	};
	typedef sp<const DirEntry> DirEntryPtr;

	UnorderedMap<string,vector<FilePtr>> _fileNames;
	UnorderedMap<string,const Google::File> _fileIds;
	VectorPtr<Google::FilePtr> LoadFiles( string_view q )noexcept(false)
	{
		//var token = Google::RefreshTokenFromSettings();
		ostringstream query;
		query << "trashed=false";
		if( q.size() )
			query << q;
		ostringstream target;
		target << "/drive/v3/files?"
			<< "pageSize=" << 500
			<< "&fields=" << Ssl::Encode( "nextPageToken, files(id, name, mimeType, parents, md5Checksum, createdTime, modifiedTime, size)" )
			<< "&q=" << Ssl::Encode( query.str() );
		string nextPageToken;
		auto pValues = make_shared<vector<Google::FilePtr>>();
		do
		{
			var target2 = nextPageToken.size()>0 ? fmt::format("{}&pageToken={}", target.str(), nextPageToken) : target.str();
			var result = Ssl::Get<FileList>( "www.googleapis.com", target2, Jde::Google::AuthorizationString() );
			pValues->reserve( pValues->size()+result.Files.size() );
			for( var& file : result.Files )
			{
				Google::FilePtr pFile = make_shared<const Google::File>( file );
				_fileIds.Set( file.Id,  pFile );
				pValues->push_back( pFile );
			}
			nextPageToken = result.NextPageToken;
		} while ( nextPageToken.size()>0 );
		
		return pValues;
	}

	const VectorPtr<FilePtr> FindName( const string& name, bool directory=false )noexcept(false)
	{
		auto pFiles = _fileNames.Find( name );
		if( !pFiles || !pFiles->size() )
		{
			ostringstream os;
			os << " and name='" << name << "'";
			if( directory )
				os << " and mimeType='application/vnd.google-apps.folder'";

			pFiles = LoadFiles( os.str() );
			if( pFiles->size() )
				_fileNames.Set( name, pFiles );
		}
		return pFiles;
	}

	Google::FilePtr FindId( const string& id )noexcept(false)
	{
		auto pFile = _fileIds.Find( id );
		if( !pFile )
		{
			ostringstream target;
			target << "/drive/v3/files/" << id
				<< "?fields=" << Ssl::Encode("id, name, parents, md5Checksum, createdTime, modifiedTime, size");
			//<< "?fields=*";

			pFile = make_shared<Google::File>( Ssl::Get<Google::File>( "www.googleapis.com", target.str(), Jde::Google::AuthorizationString()) );
			_fileIds.Set( id, pFile );
		}
		return pFile;
	}

	VectorPtr<Google::FilePtr> LoadChildren( const map<string,fs::path>& parentIds )noexcept(false)
	{
		ASSERT( parentIds.size() );
		auto pFiles = make_shared<vector<Google::FilePtr>>();
		ostringstream os;
		auto load = [&pFiles, &os]()
		{
			auto pAdditional = LoadFiles( fmt::format(" and ({})", os.str()) );//not sure have all names for _fileNames.
			pFiles->insert( pFiles->end(), pAdditional->begin(), pAdditional->end() );
			os.clear();
			os.str("");				
		};
		for( var idPath : parentIds )
		{
			if( os.tellp() )
				os << " or ";
			os << "'" << idPath.first << "' in parents";
			if( os.tellp()>1000 )
				load();
		}
		if( os.tellp() )
			load();
		for( var pFile : *pFiles )
		{
			var pParent = pFile->Parents.size()>0 ? parentIds.find( pFile->Parents[0] ) : parentIds.end();
			if( pParent!=parentIds.end() )
				pFile->Path = pParent->second/pFile->Name;
			else
				CRITICAL( "Could not find '{}' parent, ie multiple parents.", pFile->Parents.size() ? pFile->Parents[0] : "" );
		}
		return pFiles;
	}

	vector<Google::FilePtr> FindPath( const fs::path& path, bool directory=false )noexcept(false)
	{
		vector<Google::FilePtr> found;
		if( path.empty() || path.string()=="/" )
			found.push_back( make_shared<const Google::File>() );
		else
		{
			std::function<bool(const Google::File&)> where = [&path]( const Google::File& file ){ return path==file.Path; };
			auto pExisting = _fileIds.FindFirst( where );
			if( pExisting )
				found.push_back( pExisting );
			else
			{
				//UnorderedMap<string,const Google::File> _fileIds;
				//if( path.stem().string()!=path.filename().string() )
				//	DBG( "stem={}, filename={}", path.stem().string(), path.filename().string() );

				auto pFiles = FindName(path.filename().string(), directory );//
				for( auto pFile : *pFiles )
				{
					function<bool(Google::FilePtr pFile, const fs::path& current)> find;
					find = [&find]( Google::FilePtr pFile, const fs::path& current )
					{
						bool result = false;
						for( var parentId : pFile->Parents )
						{
							var pParent = FindId( parentId );
							if( pParent )
							{
								if( pParent->Path.empty() )
									pParent->Path = fs::path( pParent->Name  );
								result = !pParent->Parents.empty() && !current.empty() && pParent->Name==current.stem().string()
									? find( pParent, current.parent_path() )
									: pParent->Parents.empty() && current.empty();
							}
						}
						return result;
					};
					if( find(pFile, path.parent_path()) )
					{
						found.push_back( make_shared<const Google::File>(*pFile, path) );
						break;
					}
				}
			}
		}
		return found;
	}


	//WindowsDrive::Recursive( const fs::path& path )noexcept(false) override;
	map<string,IDirEntryPtr> GoogleDrive::Recursive( const fs::path& dir )noexcept(false)
	{
		var values = FindPath( dir );
		if( values.size()==0 )
			THROW( IOException("'{}' does not exist.", dir) );
		if( values.size()>1 )
			THROW( IOException("'{}' has '{}' entries.", dir, values.size()) );

		map<string,IDirEntryPtr> entries;
		var dirString = dir.string();
		std::function<void(vector<Google::FilePtr>)> process = [&dir, &dirString, &entries, &process]( const vector<Google::FilePtr>& files )
		{
			map<string,fs::path> parentIds;
			for( var pFile : files )
			{
				var pParent = pFile->Parents.size()>0 ? FindId( pFile->Parents[0] ) : Google::FilePtr{};
				ASSERT( pParent );
				var path = pFile->Path;
				if( pFile->IsDirectory() )
					parentIds.emplace( pFile->Id, path );
				
				var relativeDir = pFile->Path.string().substr( dirString.size()+1 );
				entries.emplace( relativeDir, make_shared<const DirEntry>(*pFile) );
			}
			
			if( parentIds.size()>0 )
			{
				auto pValues = LoadChildren( parentIds );
				process( *pValues );
			}
		};
		process( *LoadChildren(map<string,fs::path>{{values[0]->Id, dir}}) );

		return entries;
	}

	map<string,string> _mimeTypes;
	void LoadMimeTypes()noexcept(false)
	{
		var path = fs::current_path()/"mimetypes.json";
		if( !fs::exists(path) )
			THROW( EnvironmentException("Could not find '{}'", path) );
		std::ifstream is{ path };
		json j;
		is >> j;
		for( var& el : j.items() )
			_mimeTypes.emplace( el.key(), el.value() );
	}
	
	IDirEntryPtr GoogleDrive::CreateFolder( const fs::path& path, const IDirEntry& entry )
	{
		var parents = FindPath( path.parent_path() );
		if( parents.size()!=1 )
			THROW( IOException("destination {} found {} times.", path.string(), parents.size()) );
		Google::File file{ entry, parents[0]->Id };
		file.CreatedTime = TimePoint();
		json j = file;
		string body = j.dump();
		auto dir = Ssl::Send<Google::File>( "www.googleapis.com", "/drive/v3/files", body, "application/json", Jde::Google::AuthorizationString() );
		dir.Path = path;
		var pFile = make_shared<Google::File>( dir );
		_fileIds.emplace( dir.Id, pFile );
		return make_shared<DirEntry>( *pFile );
	}
		
	void Upload( const fs::path& source, const fs::path& destination )
	{
		constexpr string_view separator="b303c718d23a4d2c94405b8efd2a7fda";
		var contentType = fmt::format( "multipart/related; boundary={}", separator );
		if( _mimeTypes.size()==0 )
			LoadMimeTypes();
		auto pType = _mimeTypes.find( source.extension().string() );
		
		string mimeType{ pType!=_mimeTypes.end() ? pType->second : string("octet-stream") };
		var destinations = FindPath( destination );
		if( destinations.size()!=1 )
			THROW( IOException("destination {} found {} times.", destination.string(), destinations.size()) );
		Google::FileRequestBody request;  request.MimeType = mimeType; request.Name = request.OriginalFilename=source.filename().string();request.Parents.push_back( destinations[0]->Id );

		var pBinary = IO::FileUtilities::LoadBinary(source);

//		md5 hash;
//		hash.process_bytes( pBinary->data(), pBinary->size() );
//		md5::digest_type digest;
//		hash.get_digest( digest );
//		DBG( "md5={}", toString(digest) );

		ostringstream os;
		os << "--" << separator << std::endl
			<< "Content-Type: application/json; charset=UTF-8" << endl << endl
			<< nlohmann::json{request}[0] << endl<< endl
			<< "--" << separator << std::endl
			<< "Content-Type: " << mimeType << endl << endl;
		//DBG0( os.str() );
		os.write( pBinary->data(), pBinary->size() );
		os	<< endl << "--" << separator << "--";
		{
			std::ofstream file{"c:\\tmp\\test.txt"};
			file << os.str();
		}
		var result = Ssl::Send<Google::File>( "www.googleapis.com", "/upload/drive/v3/files?uploadType=multipart", os.str(), contentType, Jde::Google::AuthorizationString() );
//		ostringstream osDebug;
//		osDebug << nlohmann::json{result};
//		DBG0( osDebug.str() );
//		FindId( result.Id );
	}
	IDirEntryPtr  GoogleDrive::Save( const fs::path& destination, const vector<char>& bytes, const IDirEntry& dirEntry, uint retry )noexcept(false) //environmental
	{
		constexpr string_view separator="b303c718d23a4d2c94405b8efd2a7fda";
		var contentType = fmt::format( "multipart/related; boundary={}", separator );
		if( _mimeTypes.size()==0 )
			LoadMimeTypes(); //environmental
		var extension{ destination.extension().string() };
		auto pType = _mimeTypes.find( extension.size()>1 ? extension.substr(1) : extension );

		const string mimeType{ pType!=_mimeTypes.end() ? pType->second : string("application/octet-stream") };
		
		var destinations = FindPath( destination.parent_path(), true );
		if( destinations.size()!=1 )
			THROW( IOException("destination {} found {} times.", destination.parent_path().string(), destinations.size()) );

		//DBG0( destination.root_directory().string() );
		Google::FileRequestBody request;  request.MimeType = mimeType; request.CreatedTime = dirEntry.CreatedTime; request.ModifiedTime = dirEntry.ModifiedTime; request.Name = request.OriginalFilename=destination.filename().string();request.Parents.push_back( destinations[0]->Id );

		ostringstream os;
		os << "--" << separator << std::endl
			<< "Content-Type: application/json; charset=UTF-8" << endl << endl
			<< nlohmann::json{request}[0] << endl<< endl
			<< "--" << separator << std::endl
			<< "Content-Type: " << mimeType << endl << endl;
		//DBG0( os.str() );
		os.write( bytes.data(), bytes.size() );
		os	<< endl << "--" << separator << "--";
		try
		{
			var result = Ssl::Send<Google::File>( "www.googleapis.com", "/upload/drive/v3/files?uploadType=multipart", os.str(), contentType, Jde::Google::AuthorizationString() );
			return make_shared<DirEntry>( result );
		}
		catch( const IOException& e )
		{
			if( e.ErrorCode!=502 || retry>5 )
				throw e;
			WARN( "googleapis returned 502, waiting 30 seconds. retry={}", retry );
			std::this_thread::sleep_for( 30s );
			return Save( destination, bytes, dirEntry, ++retry );
		}
	}

	//VectorPtr<char> GoogleDrive::Load( const fs::path& path )noexcept(false)
	VectorPtr<char> GoogleDrive::Load( const IDirEntry& dirEntry )noexcept(false)
	{
		auto pEntry = dynamic_cast<const DirEntry*>( &dirEntry );
		sp<DirEntry> pFound;
		if( !pEntry )
		{
			var files = FindPath( dirEntry.Path );
			if( files.size()!=1 )
				THROW( IOException("found '{}' entries for '{}'.", files.size(), dirEntry.Path.string()) );
			pFound = make_shared<DirEntry>( *files[0] );
			pEntry = pFound.get();
		}
		var target{ fmt::format( "/drive/v3/files/{}?alt=media", pEntry->File.Id ) };

		var value = Ssl::Get<string>( "www.googleapis.com", target, Jde::Google::AuthorizationString() );
		return make_shared<vector<char>>( value.begin(), value.end() );
	}
	void GoogleDrive::Remove( const fs::path& path )noexcept(false)
	{
		throw Exception( "Not implemented." );//https://developers.google.com/drive/api/v3/reference/files/delete
	}
	
	void GoogleDrive::Trash( const fs::path& path )noexcept(false)
	{
		//PATCH https://www.googleapis.com/drive/v3/files/fileId
		//https://www.googleapis.com/drive/v2/files/fileId/trash
		auto files = FindPath( path );
		if( files.size()!=1 )
			THROW( Exception("Could not find '{}'. files.size()='{}'", path.string(), files.size()) );
		auto pFile = files.front();
		var target{ fmt::format("/drive/v3/files/{}", pFile->Id) };
		constexpr string_view body = "{\"trashed\":true}"sv;
		auto dir = Ssl::Send<Google::File>( "www.googleapis.com", target, body, "application/json", Jde::Google::AuthorizationString(), http::verb::patch );
	}
}
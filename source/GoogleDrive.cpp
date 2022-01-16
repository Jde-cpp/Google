#include "GoogleDrive.h"
#include "../../Ssl/source/Ssl.h"
#include "DriveTypes.h"
#include "GoogleApi.h"

#define var const auto

Jde::IO::IDrive* LoadDrive()noexcept
{
	return new Jde::IO::Drive::GoogleDrive();
}

namespace Jde::IO::Drive
{
	using Jde::Google::AccessToken;
	using Jde::IO::Drive::Google::FileList;
	using Jde::IO::Drive::Google::FilePtr;
	struct GoogleDirEntry : IDirEntry
	{
		GoogleDirEntry( const Google::File& file ):
			IDirEntry{ file.IsDirectory() ? EFileFlags::Directory : EFileFlags::None, file.Path, file.Size, file.CreatedTime, file.ModifiedTime },
			File2{ file }
		{}
		Google::File File2;
	};
	typedef sp<const GoogleDirEntry> GoogleDirEntryPtr;

	Collections::UnorderedMap<string,vector<FilePtr>> _fileNames;
	Collections::UnorderedMap<string,const Google::File> _fileIds;
	VectorPtr<FilePtr> LoadFiles( sv q, bool trashed=false, uint max=std::numeric_limits<uint>::max() )noexcept(false)
	{
		ostringstream query;
		query << "trashed=" << (trashed ? "true" : "false");
		if( q.size() )
			query << q;
		ostringstream fields;
		fields <<"nextPageToken, files(id, name, mimeType, parents, md5Checksum, createdTime, modifiedTime, size";
		if( trashed )
			fields << ", trashedTime";
		fields << ")";
		ostringstream target;
		target << "/drive/v3/files?"
			<< "pageSize=" << 500
			<< "&fields=" << Ssl::Encode( fields.str() )
			<< "&q=" << Ssl::Encode( query.str() );
		string nextPageToken;
		auto pValues = make_shared<vector<FilePtr>>();
		Try( [&]()
		{
			do
			{
				var target2 = nextPageToken.size()>0 ? format("{}&pageToken={}", target.str(), nextPageToken) : target.str();
				var result = Ssl::Get<FileList>( "www.googleapis.com", target2, Jde::Google::AuthorizationString() );
				pValues->reserve( pValues->size()+result.Files.size() );
				for( var& file : result.Files )
				{
					FilePtr pFile = make_shared<const Google::File>( file );
					_fileIds.Set( file.Id,  pFile );
					pValues->push_back( pFile );
				}
				nextPageToken = result.NextPageToken;
			} while ( nextPageToken.size()>0 && pValues->size()<max );
		});
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

	FilePtr FindId( const string& id )noexcept(false)
	{
		auto pFile = _fileIds.Find( id );
		if( !pFile )
		{
			ostringstream target;
			target << "/drive/v3/files/" << id
				<< "?fields=" << Ssl::Encode("id, name, parents, md5Checksum, createdTime, modifiedTime, size");

			pFile = make_shared<Google::File>( Ssl::Get<Google::File>( "www.googleapis.com", target.str(), Jde::Google::AuthorizationString()) );
			_fileIds.Set( id, pFile );
		}
		return pFile;
	}

	VectorPtr<FilePtr> LoadChildren( const flat_map<string,fs::path>& parentIds )noexcept(false)
	{
		ASSERT( parentIds.size() );
		auto pFiles = make_shared<vector<FilePtr>>();
		ostringstream os;
		auto load = [&pFiles, &os]()
		{
			auto pAdditional = LoadFiles( format(" and ({})", os.str()) );//not sure have all names for _fileNames.
			pFiles->insert( pFiles->end(), pAdditional->begin(), pAdditional->end() );
			os.clear();
			os.str("");
		};
		for( var& idPath : parentIds )
		{
			if( os.tellp() )
				os << " or ";
			os << "'" << idPath.first << "' in parents";
			if( os.tellp()>1000 )
				load();
		}
		if( os.tellp() )
			load();
		for( var& pFile : *pFiles )
		{
			var pParent = pFile->Parents.size()>0 ? parentIds.find( pFile->Parents[0] ) : parentIds.end();
			if( pParent!=parentIds.end() )
				pFile->Path = pParent->second/pFile->Name;
			else
				CRITICAL( "Could not find '{}' parent, ie multiple parents."sv, pFile->Parents.size() ? pFile->Parents[0] : "" );
		}
		return pFiles;
	}
	vector<FilePtr> FindPath( path path, bool directory=false )noexcept(false)
	{
		vector<FilePtr> found;
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
				auto pFiles = FindName(path.filename().string(), directory );//
				for( auto pFile : *pFiles )
				{
					function<bool(FilePtr pFile, Jde::path current)> find;
					find = [&find]( FilePtr pFile, Jde::path current )
					{
						bool result = false;
						for( var& parentId : pFile->Parents )
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

	flat_map<string,IDirEntryPtr> GoogleDrive::Recursive( path dir )noexcept(false)
	{
		var values = FindPath( dir );
		THROW_IFX( values.size()==0, IOException(dir, "does not exist.") );
		THROW_IFX( values.size()>1, IOException( SRCE_CUR, dir, "Has '{}' entries.", values.size()) );

		flat_map<string,IDirEntryPtr> entries;
		var dirString = dir.string();
		std::function<void(vector<FilePtr>)> process = [&dirString, &entries, &process]( const vector<FilePtr>& files )
		{
			flat_map<string,fs::path> parentIds;
			for( var& pFile : files )
			{
				var pParent = pFile->Parents.size()>0 ? FindId( pFile->Parents[0] ) : FilePtr{};
				ASSERT( pParent );
				var path = pFile->Path;
				if( pFile->IsDirectory() )
					parentIds.emplace( pFile->Id, path );

				var relativeDir = pFile->Path.string().substr( dirString.size()+1 );
				entries.emplace( relativeDir, make_shared<const GoogleDirEntry>(*pFile) );
			}

			if( parentIds.size()>0 )
			{
				auto pValues = LoadChildren( parentIds );
				process( *pValues );
			}
		};
		process( *LoadChildren(flat_map<string,fs::path>{{values[0]->Id, dir}}) );

		return entries;
	}

	flat_map<string,string> _mimeTypes;
	void LoadMimeTypes()noexcept(false)
	{
		var path = fs::current_path()/"mimetypes.json"; CHECK_PATH( path, SRCE_CUR );
		std::ifstream is{ path };
		nlohmann::json j;
		is >> j;
		for( var& el : j.items() )
			_mimeTypes.emplace( el.key(), el.value() );
	}

	IDirEntryPtr GoogleDrive::CreateFolder( path path, const IDirEntry& entry )
	{
		var parents = FindPath( path.parent_path() );
		THROW_IFX( parents.size()!=1, IOException(SRCE_CUR, path, "destination found {} times.", parents.size()) );
		Google::File file{ entry, parents[0]->Id };
		file.CreatedTime = TimePoint();
		nlohmann::json j = file;
		string body = j.dump();
		auto dir = Ssl::Send<Google::File>( "www.googleapis.com", "/drive/v3/files", body, "application/json", Jde::Google::AuthorizationString() );
		dir.Path = path;
		var pFile = make_shared<Google::File>( dir );
		_fileIds.emplace( dir.Id, pFile );
		return make_shared<GoogleDirEntry>( *pFile );
	}

	void Upload( path source, path destination )
	{
		constexpr sv separator="b303c718d23a4d2c94405b8efd2a7fda";
		var contentType = format( "multipart/related; boundary={}", separator );
		if( _mimeTypes.size()==0 )
			LoadMimeTypes();
		auto pType = _mimeTypes.find( source.extension().string() );

		string mimeType{ pType!=_mimeTypes.end() ? pType->second : string("octet-stream") };
		var destinations = FindPath( destination );
		THROW_IFX( destinations.size()!=1, IOException(SRCE_CUR, destination, "destination found {} times.", destinations.size()) );
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
//		ostringstream osDebug;"dbs/market/securities/arca/IAI/2019-12-10.dat.xz"
//		osDebug << nlohmann::json{result}; "/mnt/2TB/securities/arca/IAI/2019-12-10.dat.xz"
//		DBG0( osDebug.str() );
//		FindId( result.Id );
	}
	IDirEntryPtr  GoogleDrive::Save( path destination, const vector<char>& bytes, const IDirEntry& dirEntry, uint retry )noexcept(false) //environmental
	{
		Google::File fooFile;
		Google::File fooFile2{ fooFile };
		var foo =  make_shared<GoogleDirEntry>( fooFile );//dbs/market/securities/arca/GLD/2019-12-10.dat.xz

		constexpr sv separator="b303c718d23a4d2c94405b8efd2a7fda";
		var contentType = format( "multipart/related; boundary={}", separator );
		if( _mimeTypes.size()==0 )
			LoadMimeTypes(); //environmental
		var extension{ destination.extension().string() };
		auto pType = _mimeTypes.find( extension.size()>1 ? extension.substr(1) : extension );

		const string mimeType{ pType!=_mimeTypes.end() ? pType->second : string("application/octet-stream") };

		var destinations = FindPath( destination.parent_path(), true );
		THROW_IFX( destinations.size()!=1, IOException(SRCE_CUR, destination, "destination found {} times.", destinations.size()) );

		Google::FileRequestBody request;  request.MimeType = mimeType; request.CreatedTime = dirEntry.CreatedTime; request.ModifiedTime = dirEntry.ModifiedTime; request.Name = request.OriginalFilename=destination.filename().string();request.Parents.push_back( destinations[0]->Id );

		ostringstream os;
		os << "--" << separator << std::endl
			<< "Content-Type: application/json; charset=UTF-8" << endl << endl
			<< nlohmann::json{request}[0] << endl<< endl
			<< "--" << separator << std::endl
			<< "Content-Type: " << mimeType << endl << endl;

		os.write( bytes.data(), bytes.size() );
		os	<< endl << "--" << separator << "--";
		try
		{
			var result = Ssl::Send<Google::File>( "www.googleapis.com", "/upload/drive/v3/files?uploadType=multipart", os.str(), contentType, Jde::Google::AuthorizationString() );
			return make_shared<GoogleDirEntry>( result );
		}
		catch( IOException& e )
		{
			if( e.Code!=502 || retry>5 )
				throw move(e);
			WARN( "googleapis returned 502, waiting 30 seconds. retry={}"sv, retry );
			std::this_thread::sleep_for( 30s );
			return Save( destination, bytes, dirEntry, ++retry );
		}
	}

	VectorPtr<char> GoogleDrive::Load( const IDirEntry& dirEntry )noexcept(false)
	{
		auto pEntry = dynamic_cast<const GoogleDirEntry*>( &dirEntry );
		sp<GoogleDirEntry> pFound;
		if( !pEntry )
		{
			var files = FindPath( dirEntry.Path );
			THROW_IFX( files.size()!=1, IOException(SRCE_CUR, dirEntry.Path, "found '{}' entries for '{}'.", files.size()) );
			pFound = make_shared<GoogleDirEntry>( *files[0] );
			pEntry = pFound.get();
		}
		var target{ format( "/drive/v3/files/{}?alt=media", pEntry->File2.Id ) };

		var value = Ssl::Get<string>( "www.googleapis.com", target, Jde::Google::AuthorizationString() );
		return make_shared<vector<char>>( value.begin(), value.end() );
	}
	void GoogleDrive::Remove( path path )noexcept(false)
	{
		throw Exception( "Not implemented." );//https://developers.google.com/drive/api/v3/reference/files/delete
	}

	void GoogleDrive::Trash( path path )noexcept(false)
	{
		//PATCH https://www.googleapis.com/drive/v3/files/fileId
		//https://www.googleapis.com/drive/v2/files/fileId/trash
		auto files = FindPath( path ); THROW_IFX( files.size()!=1, IOException(SRCE_CUR, path, "Could not find. files.size()='{}'", files.size()) );
		auto pFile = files.front();
		var target{ format("/drive/v3/files/{}", pFile->Id) };
		constexpr sv body = "{\"trashed\":true}"sv;
		auto dir = Ssl::Send<Google::File>( "www.googleapis.com", target, body, "application/json", Jde::Google::AuthorizationString(), http::verb::patch );
	}
	void GoogleDrive::TrashDisposal( TimePoint latestDate )noexcept(false)
	{
		uint count = 0;
		ostringstream os;
		string date = ToIsoString( latestDate );
		os << " and modifiedTime<'" << date.substr( 0, date.size()-1 ) << "'";
		do
		{
			try
			{
				var pFiles = LoadFiles( os.str(), true, 10000 );
				count = pFiles->size();
				uint i=0;
				for( var& pFile : *pFiles )
				{
					var target{ format("/drive/v3/files/{}", pFile->Id) };//https://developers.google.com/drive/api/v3/reference/files/delete
					auto dir = Ssl::SendEmpty( "www.googleapis.com", target, Jde::Google::AuthorizationString(), http::verb::delete_ );
					DBG( "deleted '{}' from GDrive {}/{}"sv, pFile->Name, ++i, pFiles->size() );
				}
			}
			catch( const IException& )
			{
				std::this_thread::sleep_for( 1min );
			}
		} while( count>0 );
	}
	void GoogleDrive::Restore( sv name )noexcept(false)
	{
		uint count = 0;
		do
		{
			try
			{
				var pFiles = LoadFiles( format("and name='{}'", name), true, 10000 );
				count = pFiles->size();
				uint i=0;
				for( var& pFile : *pFiles )
				{
					var target{ format("/drive/v2/files/{}/untrash", pFile->Id) };//https://www.googleapis.com/drive/v2/files/fileId/untrash
					auto dir = Ssl::SendEmpty( "www.googleapis.com", target, Jde::Google::AuthorizationString(), http::verb::post );
					DBG( "restored '{}' from GDrive {}/{}"sv, pFile->Name, ++i, pFiles->size() );
				}
			}
			catch( const IException& )
			{
				std::this_thread::sleep_for( 1min );
			}
		} while( count>0 );
	}
}
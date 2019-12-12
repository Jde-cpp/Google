#pragma once

namespace Jde::IO{ struct IDirEntry; }

namespace Jde::IO::Drive::Google
{
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	struct File
	{
		File()noexcept;
		//File( const File& file );
		File( const File& file, const fs::path& path)noexcept;
		File( const IDirEntry& entry, string_view parentId )noexcept;
		bool IsDirectory()const noexcept{ return MimeType=="application/vnd.google-apps.folder"; }

		string Id{"root"};
		TimePoint CreatedTime;
		string Kind;
		string MimeType{"application/vnd.google-apps.folder"};
		TimePoint ModifiedTime;
		string Name;
		vector<string> Parents;
		mutable fs::path Path;
		uint Size{0};
	};
	typedef sp<const File> FilePtr;
	inline void to_json( nlohmann::json& j, const File& object )
	{
//		j = { {"id", object.Id}, {"kind", object.Kind}, {"name", object.Name}, {"mimeType", object.MimeType}, {"parents", object.Parents} , {"createdTime", DateTime(object.CreatedTime).ToIsoString()}, {"modifiedTime", DateTime(object.ModifyTime).ToIsoString()}, {"size", object.Size} };
		if( object.Id.size() )
			j["id"] = object.Id;
		if( object.CreatedTime.time_since_epoch()!=Duration::zero() )
			j["createdTime"] = DateTime(object.CreatedTime).ToIsoString();
		if( object.Kind.size() )
			j["kind"] = object.Kind;
		if( object.MimeType.size() )
			j["mimeType"] = object.MimeType;
		if( object.ModifiedTime.time_since_epoch()!=Duration::zero() )
			j["modifiedTime"] = DateTime(object.ModifiedTime).ToIsoString();
		if( object.Name.size() )
			j["name"] = object.Name;
		if( object.Parents.size() )
			j["parents"] = object.Parents;
		//if( object.OriginalFilename.size() )
		//	j["originalFilename"] = object.OriginalFilename;
		//if( object.Title.size() )
		//	j["title"] = object.Title;

	}
	inline void from_json( const nlohmann::json& j, File& object )
	{
		if( j.find("id")!=j.end() )
			j.at("id").get_to( object.Id );
		string date;
		if( j.find("createdTime")!=j.end() )
		{
			j.at("createdTime").get_to( date );
			object.CreatedTime = DateTime( date ).GetTimePoint();
		}
		if( j.find("kind")!=j.end() )
			j.at("kind").get_to( object.Kind );
		if( j.find("name")!=j.end() )
			j.at("name").get_to( object.Name );
		if( j.find("mimeType")!=j.end() )
			j.at("mimeType").get_to( object.MimeType );
		if( j.find("modifiedTime")!=j.end() )
		{
			j.at( "modifiedTime" ).get_to( date );
			object.ModifiedTime = DateTime( date ).GetTimePoint();
		}
		if( j.find("parents")!=j.end() )
			j.at("parents").get_to( object.Parents );
		if( j.find("size")!=j.end() )
		{
			string value;
			j.at("size").get_to( value );
			object.Size = stoull( value );
		}
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	struct FileRequestBody
	{
		string Id;
		TimePoint CreatedTime;
		string MimeType;
		vector<string> Parents;
		TimePoint ModifiedTime;
		string Name;
		string OriginalFilename;
		string Title;
	};
	inline void to_json( nlohmann::json& j, const FileRequestBody& object )
	{
		//nlohmann::json j;
		if( object.Id.size() )
			j["id"] = object.Id;
		if( object.CreatedTime.time_since_epoch()!=Duration::zero() )
			j["createdTime"] = DateTime(object.CreatedTime).ToIsoString();
		if( object.MimeType.size() )
			j["mimeType"] = object.MimeType;
		if( object.Parents.size() )
			j["parents"] = object.Parents;
		if( object.Name.size() )
			j["name"] = object.Name;
		if( object.ModifiedTime.time_since_epoch()!=Duration::zero() )
			j["modifiedTime"] = DateTime(object.ModifiedTime).ToIsoString();
		if( object.OriginalFilename.size() )
			j["originalFilename"] = object.OriginalFilename;
		if( object.Title.size() )
			j["title"] = object.Title;
		//j = { {"id", object.Id}, {"mimeType", object.MimeType}, {"parents", object.Parents}, {"originalFilename", object.OriginalFilename}, {"title", object.Title} };
	}
	inline void from_json( const nlohmann::json& j, FileRequestBody& object )
	{
		if( j.find("id")!=j.end() )
			j.at("id").get_to( object.Id );
		if( j.find("createdTime")!=j.end() )
		{
			string time;
			j.at("createdTime").get_to( time );
			object.CreatedTime = DateTime{ time }.GetTimePoint();
		}
		if( j.find("mimeType")!=j.end() )
			j.at("mimeType").get_to( object.MimeType );
		if( j.find("parents")!=j.end() )
			j.at("parents").get_to( object.Parents );
		if( j.find("modifiedTime")!=j.end() )
		{
			string time;
			j.at("modifiedTime").get_to( time );
			object.ModifiedTime = DateTime{ time }.GetTimePoint();
		}
		if( j.find("name")!=j.end() )
			j.at("name").get_to( object.Name );
		if( j.find("originalFilename")!=j.end() )
			j.at("originalFilename").get_to( object.OriginalFilename );
		if( j.find("title")!=j.end() )
			j.at("title").get_to( object.Title );
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	struct FileList
	{
		string Kind;
		string NextPageToken;
		bool IncompleteSearch;
		vector<File> Files;
	};
	inline void to_json( nlohmann::json& j, const FileList& object )
	{
		//nlohmann::json j2 = 
		j = { {"kind", object.Kind}, {"nextPageToken", object.NextPageToken}, {"incompleteSearch", object.IncompleteSearch}, {"files", object.Files} };
	}
	inline void from_json( const nlohmann::json& j, FileList& object )
	{
		if( j.find("kind")!=j.end() )
			j.at("kind").get_to( object.Kind );
		if( j.find("nextPageToken")!=j.end() )
			j.at("nextPageToken").get_to( object.NextPageToken );
		if( j.find("incompleteSearch")!=j.end() )
			j.at("incompleteSearch").get_to( object.IncompleteSearch );
		if( j.find("files")!=j.end() )
			j.at("files").get_to( object.Files );
	}
}
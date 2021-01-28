#pragma once

namespace Jde::Google
{
	//result from https://oauth2.googleapis.com/tokeninfo?id_token=
	struct TokenInfo
	{
		string Iss;
		string Azp;
		string Aud;
		string Email;
		bool EmailVerified{false};
		string Name;
		string PictureUrl;
		string GivenName;
		string FamilyName;
		string Locale;
		time_t Iat{0};
		time_t Expiration{0};
	};
	inline void from_json( const nlohmann::json& j, TokenInfo& token )noexcept(false)
	{
		if( j.find("iss")!=j.end() )
			j.at("iss").get_to( token.Iss );
		if( j.find("azp")!=j.end() )
			j.at("azp").get_to( token.Azp );
		if( j.find("aud")!=j.end() )
			j.at("aud").get_to( token.Aud );
		if( j.find("email")!=j.end() )
			j.at("email").get_to( token.Email );
		if( j.find("email_verified")!=j.end() )
			token.EmailVerified = j["email_verified"]=="true";
		if( j.find("name")!=j.end() )
			j.at("name").get_to( token.Name );
		if( j.find("picture")!=j.end() )
			j.at("picture").get_to( token.PictureUrl );
		if( j.find("given_name")!=j.end() )
			j.at("given_name").get_to( token.GivenName );
		if( j.find("family_name")!=j.end() )
			j.at("family_name").get_to( token.FamilyName );
		if( j.find("locale")!=j.end() )
			j.at("locale").get_to( token.Locale );
		if( j.find("iat")!=j.end() )
			token.Iat = stoi( string{j["iat"]} );
		if( j.find("exp")!=j.end() )
			token.Expiration = stoi( string{j["exp"]} );
	}

	#undef var
}
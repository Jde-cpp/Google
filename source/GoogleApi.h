#pragma once

namespace Jde::Google
{
	struct AccessToken
	{
		string Token;
		TimePoint Expiration;
		string Type;
		string Scope;
	};
	#define var const auto
	using std::chrono::seconds;
	inline void to_json( nlohmann::json& j, const AccessToken& token )
	{
		//nlohmann::json j2 = { {"access_token", token.Token}, , {"scope", token.Scope}, {"token_type", token.Type} };
		var scnds = std::chrono::duration_cast<seconds>(token.Expiration-Clock::now()).count();
		nlohmann::json j2 = { {"access_token", token.Token}, {"scope", token.Scope}, {"token_type", token.Type}, {"expires_in", scnds} };
		j = j2;
	}
	inline void from_json( const nlohmann::json& j, AccessToken& token )
	{
		if( j.find("access_token")!=j.end() )
			j.at("access_token").get_to( token.Token );
		if( j.find("expires_in")!=j.end() )
		{
			uint secs;
			j.at( "expires_in" ).get_to( secs );
			token.Expiration = Clock::now()+seconds(secs);
		}
		if( j.find("scope")!=j.end() )
			j.at("scope").get_to( token.Scope );
		if( j.find("token_type")!=j.end() )
			j.at("token_type").get_to( token.Type );
	}
	
	extern shared_ptr<Jde::Settings::Container> GoogleSettingsPtr;
	const AccessToken& RefreshToken( string_view refreshToken, string_view clientId, string_view secret )noexcept(false);
	const AccessToken& RefreshTokenFromSettings()noexcept(false);
	string AuthorizationString();

	#undef var
}
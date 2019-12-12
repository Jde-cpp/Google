#include "GoogleApi.h"
#define var const auto

namespace Jde
{
	map<string,Google::AccessToken> _tokens; shared_mutex _tokenMutex;
	shared_ptr<Jde::Settings::Container> Google::GoogleSettingsPtr;

	const Google::AccessToken& Google::RefreshToken( string_view refreshToken, string_view clientId, string_view secret )noexcept(false)
	{
		const string body{ fmt::format("refresh_token={}&grant_type=refresh_token&client_id={}&client_secret={}", refreshToken, clientId, secret) };
		unique_lock l(_tokenMutex);
		auto pExisting = _tokens.find(body);
		if( pExisting!=_tokens.end() && pExisting->second.Expiration>Clock::now()+30s )
			return pExisting->second;

		var& result = _tokens[body] = Ssl::Send<AccessToken>( "oauth2.googleapis.com", "/token", body );
		DBG( "received token expires in '{}' minutes.", duration_cast<std::chrono::minutes>(result.Expiration-Clock::now()).count() );
		const fs::path path{ GoogleSettingsPtr->Get<fs::path>( "saveFile" ) };
		nlohmann::json j;
		j["key"] = body;
		j["time"] = DateTime(Clock::now()).ToIsoString();
		j["token"] = result;
		std::ofstream os{path};
		os << j;

		return result;
	}

	const Google::AccessToken& Google::RefreshTokenFromSettings()noexcept(false)
	{
		if( !GoogleSettingsPtr )
		{
			GoogleSettingsPtr = Settings::Global().SubContainer( "google" );
			// if( !GoogleSettingsPtr )
			// 	THROW( EnvironmentException("Could not find google settings.") );
			const fs::path path{ GoogleSettingsPtr->Get<fs::path>( "saveFile" ) };
			if( fs::exists(path) )
			{
				try
				{
					const string text{ Jde::IO::FileUtilities::ToString(path) };
					var j = nlohmann::json::parse( text );
					unique_lock l{_tokenMutex};
					var time = j["time"].get<string>();
					var key = j["key"].get<string>();
					var tokenJson = j.at( "token" );
					auto token2{ tokenJson.get<AccessToken>() };
					var delta = Clock::now()-DateTime( time ).GetTimePoint();
					token2.Expiration-=delta;
					DBG( "Adding saved token expires in {} minutes", duration_cast<std::chrono::minutes>(token2.Expiration-Clock::now()).count() );
					_tokens.emplace( key, token2 );
				}
				catch( const std::exception& e )
				{
					DBG( "Could not parse '{}' - {}", path.string(), e.what() );
				}
			}
		}
		const string refreshToken{GoogleSettingsPtr->Get<string>("refreshToken") };
		const string clientId{GoogleSettingsPtr->Get<string>("clientId") };
		const string secret{GoogleSettingsPtr->Get<string>("secret") };

		return RefreshToken( refreshToken, clientId, secret );
	}
	string Google::AuthorizationString()
	{
		var token = RefreshTokenFromSettings();
		return fmt::format( "{} {}", token.Type, token.Token );
	}

}
#include "GoogleApi.h"
#include "../../Ssl/source/Ssl.h"
#define var const auto

namespace Jde
{
	flat_map<string,Google::AccessToken> _tokens; shared_mutex _tokenMutex;
	sp<Jde::Settings::Container> Google::GoogleSettingsPtr;

	const Google::AccessToken& Google::RefreshToken( sv refreshToken, sv clientId, sv secret )noexcept(false)
	{
		const string body{ fmt::format("refresh_token={}&grant_type=refresh_token&client_id={}&client_secret={}", refreshToken, clientId, secret) };
		unique_lock l(_tokenMutex);
		auto pExisting = _tokens.find(body);
		if( pExisting!=_tokens.end() && pExisting->second.Expiration>Clock::now()+30s )
			return pExisting->second;

		var& result = _tokens[body] = Ssl::Send<AccessToken>( "oauth2.googleapis.com", "/token", body );
		DBG( "received token expires in '{}' minutes."sv, duration_cast<std::chrono::minutes>(result.Expiration-Clock::now()).count() );
		const fs::path path{ GoogleSettingsPtr->Getɛ<fs::path>( "saveFile" ) };
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
			const fs::path path{ GoogleSettingsPtr->Getɛ<fs::path>( "saveFile" ) };
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
					DBG( "Adding saved token expires in {} minutes"sv, duration_cast<std::chrono::minutes>(token2.Expiration-Clock::now()).count() );
					_tokens.emplace( key, token2 );
				}
				catch( const std::exception& e )
				{
					DBG( "Could not parse '{}' - {}"sv, path.string(), e.what() );
				}
			}
		}
		const string refreshToken{ GoogleSettingsPtr->Getɛ<string>("refreshToken") };
		const string clientId{ GoogleSettingsPtr->Getɛ<string>("clientId") };
		const string secret{ GoogleSettingsPtr->Getɛ<string>("secret") };

		return RefreshToken( refreshToken, clientId, secret );
	}
	string Google::AuthorizationString()
	{
		var token = RefreshTokenFromSettings();
		return fmt::format( "{} {}", token.Type, token.Token );
	}
}
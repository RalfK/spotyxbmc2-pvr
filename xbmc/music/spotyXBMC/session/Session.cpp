/*
 spotyxbmc2 - A project to integrate Spotify into XBMC
 Copyright (C) 2011  David Erenger

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For contact with the author:
 david.erenger@gmail.com
 */

#include "Session.h"
#include "../../../../appkey.h"
#include <pthread.h>
#include <string>

#include "../artist/ArtistStore.h"
#include "../track/TrackStore.h"
#include "../album/AlbumStore.h"
#include "../thumb/ThumbStore.h"
#include "../radio/RadioHandler.h"
#include "../search/SearchHandler.h"
#include "../XBMCUpdater.h"

namespace addon_music_spotify {

  Session::Session() {
    m_sessionCallbacks = new SessionCallbacks();
    m_isEnabled = false;
    m_isLoggedOut = false;
    m_session = NULL;
    m_playlists = NULL;
    m_nextEvent = 0;
    m_lock = false;

  }

  void Session::deInit() {
    m_isEnabled = false;
  }

  Session::~Session() {
    delete m_sessionCallbacks;
  }

  bool Session::enable() {
    if (isEnabled()) return true;

    m_isEnabled = true;

    lock();
    m_bgThread = new BackgroundThread();
    m_bgThread->Create(true);
    unlock();

    return true;
  }

  bool Session::processEvents() {
    sp_session_process_events(m_session, &m_nextEvent);
    return true;
  }

  bool Session::connect() {
    if (!m_session) {
      sp_session_config config;
      Logger::printOut("Creating session");

      config.api_version = SPOTIFY_API_VERSION;
      config.cache_location = Settings::getCachePath().c_str();
      config.settings_location = Settings::getCachePath().c_str();
      config.application_key = g_appkey;
      config.application_key_size = g_appkey_size;
      config.user_agent = "spotify-for-XBMC2000";
      config.callbacks = &m_sessionCallbacks->getCallbacks();
      config.compress_playlists = true;
      config.dont_save_metadata_for_playlists = false;
      config.initially_unload_playlists = false;

      sp_error error = sp_session_create(&config, &m_session);

      if (SP_ERROR_OK != error) {
        Logger::printOut("Failed to create session: error:");
        m_session = NULL;
        return false;
      }

      //set high bitrate
      if (Settings::useHighBitrate()) sp_session_preferred_bitrate(m_session, SP_BITRATE_320k);

      sp_session_set_connection_type(m_session, SP_CONNECTION_TYPE_WIRED);
      sp_session_set_connection_rules(m_session, SP_CONNECTION_RULE_NETWORK);

      sp_session_login(m_session, Settings::getUserName().c_str(), Settings::getPassword().c_str());
      m_isEnabled = true;
      Logger::printOut("Logged in, returning");
      return true;
    }
    return false;
  }

  Session* Session::m_instance = 0;
  Session *Session::getInstance() {
    return m_instance ? m_instance : (m_instance = new Session);
  }

  bool Session::isReady() {
    return m_isEnabled;
  }

  bool Session::disConnect() {
    Logger::printOut("Logging out");

    delete m_playlists;
    m_playlists = NULL;
    Logger::printOut("cleaned playlists");

    SearchHandler::deInit();
    Logger::printOut("cleaned search");

    RadioHandler::deInit();
    Logger::printOut("cleaned radios");

    ArtistStore::deInit();
    Logger::printOut("cleaned artists");

    AlbumStore::deInit();
    Logger::printOut("cleaned albums");

    TrackStore::deInit();
    Logger::printOut("cleaned tracks");

    ThumbStore::deInit();
    Logger::printOut("cleaned thumbs");

    //TODO FIX THE LOGOUT... Why is it crashing on logout?
    m_isLoggedOut = false;
    sp_session_logout(m_session);

    Logger::printOut("logged out waiting for callback");
    while (!m_isLoggedOut) {
      processEvents();
    }
    Logger::printOut("logged out");
    sp_session_release(m_session);
    Logger::printOut("cleaned session");

    return true;
  }

  void Session::loggedIn() {
    m_playlists = new PlaylistStore();
    //update the menu so the radio appears
    XBMCUpdater::updateMenu();
  }

  void Session::loggedOut() {
    Logger::printOut("logged out session");
    m_isLoggedOut = true;
  }

  bool Session::lock() {
    if (m_lock)
      return false;
    else
      m_lock = true;
    return true;
  }

  bool Session::unlock() {
    m_lock = false;
    return true;
  }

} /* namespace addon_music_spotify */

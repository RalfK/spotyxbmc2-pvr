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

#include "SessionCallbacks.h"
#include "../album/AlbumStore.h"
#include "../player/PlayerHandler.h"
#include "Session.h"

namespace addon_music_spotify {

  SessionCallbacks::SessionCallbacks() {
      m_dll = new DllLibspotify();
      m_dll->Load();

      memset(&m_callbacks, 0, sizeof(m_callbacks));

    m_callbacks.connection_error = &cb_connectionError;
    m_callbacks.logged_out = &cb_loggedOut;
    m_callbacks.logged_in = &cb_loggedIn;
    m_callbacks.notify_main_thread = &cb_notifyMainThread;
    m_callbacks.music_delivery = &PlayerHandler::cb_musicDelivery;
    m_callbacks.credentials_blob_updated = &cb_credentialsBlobUpdated;
    m_callbacks.log_message = &cb_logMessage;
    m_callbacks.end_of_track = &PlayerHandler::cb_endOfTrack;
  }

  SessionCallbacks::~SessionCallbacks() {
      delete m_dll, m_dll = NULL;
  }

  void SessionCallbacks::cb_credentialsBlobUpdated(sp_session *session, const char *blob) {
    Session::getInstance()->updateCredentialsBlob(blob);
  }

  void SessionCallbacks::cb_connectionError(sp_session *session, sp_error error) {
  }

  void SessionCallbacks::cb_loggedIn(sp_session *session, sp_error error) {
      DllLibspotify *m_dll = new DllLibspotify();
      m_dll->Load();
    if (error == SP_ERROR_OK) {
      Session::getInstance()->loggedIn();
      Logger::printOut("Logged in!");
    } else {
      Logger::printOut("Error while logging in!");
      Logger::printOut(m_dll->sp_error_message(error));
    }

    delete m_dll, m_dll = NULL;
  }

  void SessionCallbacks::cb_loggedOut(sp_session *session) {
    Logger::printOut("Logged out callback!");
    Session::getInstance()->loggedOut();
  }

  void SessionCallbacks::cb_notifyMainThread(sp_session *session) {
    Session::getInstance()->notifyMainThread();
  }

  void SessionCallbacks::cb_logMessage(sp_session *session, const char *data) {
    Logger::printOut((char*) data);
  }

  void SessionCallbacks::cb_topListAritstsComplete(sp_toplistbrowse *result, void *userdata) {
  }

  void SessionCallbacks::cb_topListAlbumsComplete(sp_toplistbrowse *result, void *userdata) {
  }

  void SessionCallbacks::cb_topListTracksComplete(sp_toplistbrowse *result, void *userdata) {
  }

  void SessionCallbacks::cb_artistBrowseComplete(sp_artistbrowse *result, void *userdata) {
  }

} /* namespace addon_music_spotify */


/* $Id: settings.hpp 49342 2011-05-02 18:52:36Z crab $ */
/*
   Copyright (C) 2007 - 2011 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file src/gui/widgets/settings.hpp
 * This file contains the settings handling of the widget library.
 */

#ifndef GUI_WIDGETS_SETTING_HPP_INCLUDED
#define GUI_WIDGETS_SETTING_HPP_INCLUDED

#include "gui/auxiliary/widget_definition/window.hpp"

#include <string>
#include <vector>

namespace gui2 {

/** Do we wish to use the new library or not. */
extern bool new_widgets;

enum twindow_type {
	TITLE_SCREEN,            /**< The main title screen of the game. */
	ADDON_CONNECT,           /**< The addon server connection dialog. */
	ADDON_LIST,              /**< The addon list dialog. */
	CAMPAIGN_SELECTION,      /**< The campaign selection dialog. */
	CHAT_LOG,                /**< The chat log window. */
	LANGUAGE_SELECTION,      /**< The language selection dialog. */
	MESSAGE,                 /**< A generic message dialog. */
	TRANSIENT_MESSAGE,       /**< A transient message dialog. */
	WML_MESSAGE_LEFT,        /**<
	                          * A WML message dialog with the portrait on
	                          * the left side.
	                          */
	WML_MESSAGE_RIGHT,       /**<
	                          * A WML message dialog with the portrait on
	                          * the right side.
	                          */
	MAPGEN_SETTINGS,          /**< The mp creation dialog. */
	MP_CONNECT,              /**< The mp server connection dialog. */
	MP_METHOD_SELECTION,     /**<
	                          * The dialog which allows you to choose the kind
	                          * mp game the user wants to play.
	                          */
	MP_SERVER_LIST,          /**< The mp server list dialog. */
	MP_LOGIN,                /**< The mp login dialog. */
	MP_CMD_WRAPPER,          /**< graphical front-end to various mp commands*/
	MP_CREATE_GAME,          /**< The mp creation dialog. */
	MP_GAME_CONNECT,          /**< The mp game connect(host) dialog. */
	MP_GAME_WAIT,          /**< The mp game connect(host) dialog. */
	MP_LEADER_PREVIEW,          /**< The mp game connect(host) dialog. */
#ifndef DISABLE_EDITOR
	EDITOR_NEW_MAP,          /**< New map dialog. */
	EDITOR_GENERATE_MAP,     /**< Editor random map genarator dialog. */
	EDITOR_RESIZE_MAP,       /**< Editor resize map dialog. */
	EDITOR_SETTINGS,         /**< Editor settings dialog. */
#endif
	GAME_LOAD,				 /**< Load game dialog. */
	GAME_DELETE,			 /**< Confirm delete dialog. */
	GAME_SAVE,				 /**< Save game dialog. */
	GAME_SAVE_MESSAGE,		 /**< Save game dialog with additional message. */
	GAME_SAVE_OOS,			 /**< Save game dialog for processing OOS. */
	LOBBY_MAIN,               /**< Main MP lobby screen */
	LOBBY_PLAYER_INFO,        /**< MP lobby player info dialog */
	PREFERENCES,			 /**< preferences dialog */
	UNIT_CREATE,			 /**< Debug-mode unit creation dialog */
	UNIT_ATTACK,			 /**< Unit attack dialog */
	UNIT_ADVANCE,			 /**< Unit advance dialog */
	UNIT_LIST,			     /**< Unit list dialog */
	UNIT_RECRUIT,			 /**< Unit recruit dialog */
	UNIT_RECALL,			 /**< Unit recall dialog */
	FORMULA_DEBUGGER,         /**< Formula debugger dialog */
	GAMESTATE_INSPECTOR,      /**< Visual inspector of the game state (WML variables, events)*/
	TOUCH_GAME_MENU,			/**< game touch menu */
	COUNT                    /**<
	                          * The last one to hold the number of items and as
	                          * sentinel.
	                          */
};

const std::string& get_id(const twindow_type window_type);


	tresolution_definition_ptr get_control(
		const std::string& control_type, const std::string& definition);

	std::vector<twindow_builder::tresolution>::const_iterator
		get_window_builder(const std::string& type);

	/** Loads the setting for the theme. */
	void load_settings();

	/** This namespace contains the 'global' settings. */
	namespace settings {

		/**
		 * The screen resolution should be available for all widgets since
		 * their drawing method will depend on it.
		 */
		extern unsigned screen_width;
		extern unsigned screen_height;

		/**
		 * The size of the map area, if not available equal to the screen
		 * size.
		 */
		extern unsigned gamemap_width;
		extern unsigned gamemap_height;

		/** These are copied from the active gui. */
		extern unsigned popup_show_delay;
		extern unsigned popup_show_time;
		extern unsigned help_show_time;
		extern unsigned double_click_time;
		extern unsigned repeat_button_repeat_time;

		extern std::string sound_button_click;
		extern std::string sound_toggle_button_click;
		extern std::string sound_toggle_panel_click;
		extern std::string sound_slider_adjust;
	}

} // namespace gui2

#endif


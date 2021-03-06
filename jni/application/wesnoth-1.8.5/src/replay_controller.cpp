/* $Id: replay_controller.cpp 48834 2011-03-10 18:07:57Z dragonking $ */
/*
   Copyright (C) 2006 - 2011 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playlevel Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "game_end_exceptions.hpp"
#include "game_events.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "replay.hpp"
#include "replay_controller.hpp"
#include "resources.hpp"
#include "savegame.hpp"

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)

static lg::log_domain log_replay("replay");
#define DBG_REPLAY LOG_STREAM(debug, log_replay)
#define LOG_REPLAY LOG_STREAM(info, log_replay)

LEVEL_RESULT play_replay_level(const config& game_config,
		const config* level, CVideo& video, game_state& state_of_game)
{
	try{
		const int ticks = SDL_GetTicks();
		const int num_turns = atoi((*level)["turns"].c_str());
		DBG_NG << "creating objects... " << (SDL_GetTicks() - ticks) << "\n";
		replay_controller replaycontroller(*level, state_of_game, ticks, num_turns, game_config, video);
		DBG_NG << "created objects... " << (SDL_GetTicks() - replaycontroller.get_ticks()) << "\n";
		const events::command_disabler disable_commands;

		//replay event-loop
		for (;;){
			replaycontroller.play_slice();
		}
	}
	catch(end_level_exception&){
		DBG_NG << "play_replay_level: end_level_exception\n";
	}

	return VICTORY;
}

replay_controller::replay_controller(const config& level,
		game_state& state_of_game, const int ticks, const int num_turns,
		const config& game_config, CVideo& video) :
	play_controller(level, state_of_game, ticks, num_turns, game_config, video, false),
	teams_start_(),
	gamestate_start_(state_of_game),
	units_start_(),
	tod_manager_start_(level, num_turns, &state_of_game),
	current_turn_(1),
	delay_(0),
	is_playing_(false),
	show_everything_(false),
	show_team_(state_of_game.classification().campaign_type == "multiplayer" ? 0 : 1),
	buttons_()
{
	init();
	gamestate_start_ = gamestate_;
}

replay_controller::~replay_controller()
{
	//YogiHH
	//not absolutely sure if this is needed, but it makes me feel a lot better ;-)
	//feel free to delete this if it is not necessary
	gui_->get_theme().theme_reset_event().detach_handler(this);
	gui_->complete_redraw_event().detach_handler(this);
}

void replay_controller::init(){
	DBG_REPLAY << "in replay_controller::init()...\n";

	//guarantee the cursor goes back to 'normal' at the end of the level
	const cursor::setter cursor_setter(cursor::NORMAL);
	init_replay_display();

	fire_prestart(true);
	init_gui();
	statistics::fresh_stats();
	set_victory_when_enemies_defeated(
		utils::string_bool(level_["victory_when_enemies_defeated"], true));

	DBG_REPLAY << "first_time..." << (recorder.is_skipping() ? "skipping" : "no skip") << "\n";

	fire_start(!loading_game_);
	update_gui();

	units_start_ = units_;
	teams_start_ = teams_;
}

void replay_controller::init_gui(){
	DBG_NG << "Initializing GUI... " << (SDL_GetTicks() - ticks_) << "\n";
	play_controller::init_gui();

	if (show_team_)
		gui_->set_team(show_team_ - 1, show_everything_);
	else
		gui_->set_team(0, show_everything_);

	gui_->scroll_to_leader(units_, player_number_, display::WARP);
	update_locker lock_display((*gui_).video(),false);
	for(std::vector<team>::iterator t = teams_.begin(); t != teams_.end(); ++t) {
		t->reset_objectives_changed();
	}
	
	buttons_.update(gui_);
}

void replay_controller::init_replay_display(){
	DBG_REPLAY << "initializing replay-display... " << (SDL_GetTicks() - ticks_) << "\n";

	rebuild_replay_theme();
	gui_->get_theme().theme_reset_event().attach_handler(this);
	gui_->complete_redraw_event().attach_handler(this);
	DBG_REPLAY << "done initializing replay-display... " << (SDL_GetTicks() - ticks_) << "\n";
}

void replay_controller::rebuild_replay_theme()
{
	const config &theme_cfg = get_theme(game_config_, level_["theme"]);
	if (const config &res = theme_cfg.child("resolution"))
	{
		if (const config &replay_theme_cfg = res.child("replay"))
			gui_->get_theme().modify(replay_theme_cfg);
		gui_->get_theme().modify_label("time-icon", _ ("current local time"));
		//Make sure we get notified if the theme is redrawn completely. That way we have
		//a chance to restore the replay controls of the theme as well.
		gui_->invalidate_theme();
	}
}


void replay_controller::replay_buttons_wrapper::update(boost::scoped_ptr<game_display>& gui_)
{
 	play_button_ = gui_->find_button("button-playreplay");
	stop_button_ = gui_->find_button("button-stopreplay");
	reset_button_ = gui_->find_button("button-resetreplay");
	play_turn_button_ = gui_->find_button("button-nextturn");
	play_side_button_ = gui_->find_button("button-nextside");
	
	//check if we have all buttons - if someone messed with theme then some buttons may be missing
	//if any of the buttons is missing, we just disable everything
	if( !play_button_ || !stop_button_ || !reset_button_ || !play_turn_button_ || !play_side_button_ ) {
	 
		is_valid_ = false;
		enabled_buttons_ = 0;
	} else {
		is_valid_ = true;
		if( enabled_buttons_ == 0)
			enabled_buttons_ = PLAY_BUTTON_ENABLED | PLAY_TURN_BUTTON_ENABLED | PLAY_SIDE_BUTTON_ENABLED;
	}
	
	update_buttons_states();
}

void replay_controller::replay_buttons_wrapper::update_buttons_states()
{
	if( enabled_buttons_ & PLAY_BUTTON_ENABLED ) {
		play_button_->enable(true);
	} else
		play_button_->enable(false);
	
	if( enabled_buttons_ & STOP_BUTTON_ENABLED ) {
		stop_button_->enable(true);
	} else
		stop_button_->enable(false);
	
	if( enabled_buttons_ & RESET_BUTTON_ENABLED ) {
		reset_button_->enable(true);
	} else
		reset_button_->enable(false);		
	
	if( enabled_buttons_ & PLAY_TURN_BUTTON_ENABLED ) {
		play_turn_button_->enable(true);
	} else
		play_turn_button_->enable(false);
	
	if( enabled_buttons_ & PLAY_SIDE_BUTTON_ENABLED ) {
		play_side_button_->enable(true);
	} else
		play_side_button_->enable(false);	
}

void replay_controller::replay_buttons_wrapper::playback_should_start() 
{
	if( !is_valid_ )
		return;
	
	play_button_->enable(false);
	stop_button_->enable(true);
	reset_button_->enable(false);
	play_turn_button_->enable(false);
	play_side_button_->enable(false);
	
	enabled_buttons_ = STOP_BUTTON_ENABLED;
}

void replay_controller::replay_buttons_wrapper::playback_should_stop(bool is_playing) 
{
	if( !is_valid_)
		return;
	
	if( !recorder.at_end() ) {
		
		enabled_buttons_ = PLAY_BUTTON_ENABLED | RESET_BUTTON_ENABLED | PLAY_TURN_BUTTON_ENABLED | PLAY_SIDE_BUTTON_ENABLED;	
		
		update_buttons_states();
	
		play_button_->release();
		play_turn_button_->release();
		play_side_button_->release();
	} else {
		enabled_buttons_ = RESET_BUTTON_ENABLED;
		
		update_buttons_states();
	}
	
	if( !is_playing ) {
		//user interrupted
		stop_button_->release();
	}	
}

void replay_controller::replay_buttons_wrapper::reset_buttons() 
{
	if( !is_valid_ )
		return;
	
	enabled_buttons_ = PLAY_BUTTON_ENABLED | PLAY_TURN_BUTTON_ENABLED | PLAY_SIDE_BUTTON_ENABLED;
	
	stop_button_->release();
	reset_button_->release();
	
	update_buttons_states();
}

void replay_controller::reset_replay(){
	gui_->clear_chat_messages();
	is_playing_ = false;
	player_number_ = 1;
	current_turn_ = 1;
	skip_replay_ = false;
	tod_manager_= tod_manager_start_;
	recorder.start_replay();
	recorder.set_skip(false);
	units_ = units_start_;
	gamestate_ = gamestate_start_;
	teams_ = teams_start_;
	statistics::fresh_stats();
	if (events_manager_ ){
		// NOTE: this double reset is required so that the new
		// instance of game_events::manager isn't created before the
		// old manager is actually destroyed (triggering an assertion
		// failure)
		events_manager_.reset();
		events_manager_.reset(new game_events::manager(level_));
	}

	gui_->new_turn();
	gui_->invalidate_game_status();
	events::raise_draw_event();
	(*gui_).invalidate_all();
	(*gui_).draw();

	fire_prestart(true);
	fire_start(!loading_game_);
	gui_->new_turn();
	gui_->invalidate_game_status();
	events::raise_draw_event();
	(*gui_).invalidate_all();
	(*gui_).draw();
	gui_->set_team(player_number_-1, show_everything_);
	//gui_->scroll_to_leader(units_, player_number_,game_display::ONSCREEN,false);

	buttons_.reset_buttons();
}

void replay_controller::stop_replay(){
	is_playing_ = false;
}

void replay_controller::replay_next_turn(){
	is_playing_ = true;
	buttons_.playback_should_start();
	
	play_turn();

 	if (!skip_replay_ || !is_playing_){
		gui_->scroll_to_leader(units_, player_number_,game_display::ONSCREEN,false);
	}

	buttons_.playback_should_stop(is_playing_);
}

void replay_controller::replay_next_side(){
	is_playing_ = true;
	buttons_.playback_should_start();
	
	play_side(player_number_ - 1, false);

	if (static_cast<size_t>(player_number_) > teams_.size()) {
		player_number_ = 1;
		current_turn_++;
	}

	if (!skip_replay_ || !is_playing_) {
		gui_->scroll_to_leader(units_, player_number_,game_display::ONSCREEN,false);
	}

	buttons_.playback_should_stop(is_playing_);
}

void replay_controller::process_oos(const std::string& msg) const
{
	if (game_config::ignore_replay_errors) return;

	/** @todo FIXME: activate translation support after string freeze */
	std::stringstream message;
	message << "The replay is corrupt/out of sync. It might not make much sense to continue. Do you want to save the game?";
	message << "\n\nError details:\n\n" << msg;

	savegame::oos_savegame save(to_config());
	save.save_game_interactive(resources::screen->video(), message.str(), gui::YES_NO); // can throw end_level_exception
}

void replay_controller::replay_show_everything(){
	show_everything_ = true;
	show_team_ = 0;
	update_teams();
	update_gui();
}

void replay_controller::replay_show_each(){
	show_everything_ = false;
	show_team_ = 0;
	update_teams();
	update_gui();
}

void replay_controller::replay_show_team1(){
	show_everything_ = false;
	show_team_ = 1;
	update_teams();
	update_gui();
}

void replay_controller::replay_skip_animation(){
	skip_replay_ = !skip_replay_;
	recorder.set_skip(skip_replay_);
}

void replay_controller::play_replay(){

	if (recorder.at_end()){
		return;
	}

	try{
		is_playing_ = true;
		buttons_.playback_should_start();

		DBG_REPLAY << "starting main loop\n" << (SDL_GetTicks() - ticks_) << "\n";
		for(; !recorder.at_end() && is_playing_; first_player_ = 1) {
			play_turn();
		} //end for loop
		
		if (!is_playing_) {
			gui_->scroll_to_leader(units_, player_number_,game_display::ONSCREEN,false);
		}
	}
	catch(end_level_exception& e){
		if (e.result == QUIT) { throw e; }
	}
	
	buttons_.playback_should_stop(is_playing_);
}

void replay_controller::play_turn(){

	LOG_REPLAY << "turn: " << current_turn_ << "\n";

	gui_->new_turn();
	gui_->invalidate_game_status();
	events::raise_draw_event();

	bool last_team = false;

	while ( (!last_team) && (!recorder.at_end()) && is_playing_ ){
		last_team = static_cast<size_t>(player_number_) == teams_.size();
		play_side(player_number_ - 1, false);
		play_slice();
	}
}

void replay_controller::play_side(const unsigned int /*team_index*/, bool){

	DBG_REPLAY << "Status turn number: " << turn() << "\n";
	DBG_REPLAY << "Replay_Controller turn number: " << current_turn_ << "\n";
	DBG_REPLAY << "Player number: " << player_number_ << "\n";

	try{
		// If a side is empty skip over it.
		if (!current_team().is_empty()) {
			statistics::reset_turn_stats(current_team().save_id());

			play_controller::init_side(player_number_ - 1, true);

			DBG_REPLAY << "doing replay " << player_number_ << "\n";
			do_replay(player_number_);

			finish_side_turn();

			// This is necessary for replays in order to show possible movements.
			for (unit_map::iterator uit = units_.begin(); uit != units_.end(); ++uit) {
				if (uit->second.side() != player_number_) {
					uit->second.new_turn();
				}
			}
		}

		player_number_++;

		if (static_cast<size_t>(player_number_) > teams_.size()) {
			next_turn();
			finish_turn();
			player_number_ = 1;
			current_turn_++;
			gui_->new_turn();
		}

		update_teams();
		update_gui();
	}
	catch(end_level_exception& e){
		//VICTORY/DEFEAT end_level_exception shall not return to title screen
		if ((e.result != VICTORY) && (e.result != DEFEAT)) { throw e; }
	}
}

void replay_controller::update_teams(){
	int next_team = player_number_;
	if(static_cast<size_t>(next_team) > teams_.size()) {
		next_team = 1;
	}

	if (!show_team_) {
 		gui_->set_team(next_team - 1, show_everything_);
	} else {
		gui_->set_team(show_team_ - 1, show_everything_);
	}

	::clear_shroud(next_team);
	
	gui_->set_playing_team(next_team - 1);
	gui_->invalidate_all();
}

void replay_controller::update_gui(){
	(*gui_).recalculate_minimap();
	(*gui_).redraw_minimap();
	(*gui_).invalidate_all();
	events::raise_draw_event();
	(*gui_).draw();
}

void replay_controller::preferences(bool use_classic){
	play_controller::preferences(use_classic);
	update_gui();
}

void replay_controller::show_statistics(){
	menu_handler_.show_statistics(gui_->playing_team()+1);
}

void replay_controller::handle_generic_event(const std::string& name){
	
	if( name == "completely_redrawn" ) {
		buttons_.update(gui_);
		
		gui::button* skip_animation_button = gui_->find_button("skip-animation");
		
		skip_animation_button->set_check(skip_replay_);
	} else {
		rebuild_replay_theme();
	}
}

bool replay_controller::can_execute_command(hotkey::HOTKEY_COMMAND command, int index) const
{
	bool result = play_controller::can_execute_command(command,index);

	switch(command) {

	//commands we can always do
	case hotkey::HOTKEY_PLAY_REPLAY:
	case hotkey::HOTKEY_RESET_REPLAY:
	case hotkey::HOTKEY_STOP_REPLAY:
	case hotkey::HOTKEY_REPLAY_NEXT_TURN:
	case hotkey::HOTKEY_REPLAY_NEXT_SIDE:
	case hotkey::HOTKEY_REPLAY_SHOW_EVERYTHING:
	case hotkey::HOTKEY_REPLAY_SHOW_EACH:
	case hotkey::HOTKEY_REPLAY_SHOW_TEAM1:
	case hotkey::HOTKEY_REPLAY_SKIP_ANIMATION:
	case hotkey::HOTKEY_SAVE_GAME:
	case hotkey::HOTKEY_SAVE_REPLAY:
	case hotkey::HOTKEY_CHAT_LOG:
		return true;

	default:
		return result;
	}
}

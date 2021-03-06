
/* $Id: engine.hpp 48450 2011-02-08 20:55:18Z mordante $ */
/*
   Copyright (C) 2009 - 2011 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * AI Support engine - creating specific ai components from config
 * @file ai/composite/engine.hpp
 */

#ifndef AI_COMPOSITE_ENGINE_HPP_INCLUDED
#define AI_COMPOSITE_ENGINE_HPP_INCLUDED

#include "../../global.hpp"

#include "component.hpp"
#include "../contexts.hpp"
#include "../game_info.hpp"
#include "../../config.hpp"
#include <algorithm>
#include <iterator>
#include <vector>

//============================================================================

namespace ai {

class rca_context;
class ai_context;
class component;

class engine : public component {
public:
	engine( readonly_context &context, const config &cfg );


	virtual ~engine();


	static void parse_aspect_from_config( readonly_context &context, const config &cfg, const std::string &id, std::back_insert_iterator<std::vector< aspect_ptr > > b );


	static void parse_goal_from_config( readonly_context &context, const config &cfg, std::back_insert_iterator<std::vector< goal_ptr > > b );


	static void parse_candidate_action_from_config( rca_context &context, const config &cfg, std::back_insert_iterator<std::vector< candidate_action_ptr > > b );


	static void parse_engine_from_config( readonly_context &context, const config &cfg, std::back_insert_iterator<std::vector< engine_ptr > > b );


	static void parse_stage_from_config( ai_context &context, const config &cfg, std::back_insert_iterator<std::vector< stage_ptr > > b );


	//do not override that method in subclasses which cannot create aspects
	virtual void do_parse_aspect_from_config( const config &cfg, const std::string &id, std::back_insert_iterator< std::vector< aspect_ptr> > b );


	//do not override that method in subclasses which cannot create candidate_actions
	virtual void do_parse_candidate_action_from_config( rca_context &context, const config &cfg, std::back_insert_iterator<std::vector< candidate_action_ptr > > b );

	//do not override that method in subclasses which cannot create goals
	virtual void do_parse_goal_from_config( const config &cfg, std::back_insert_iterator<std::vector< goal_ptr > > b );

	//do not override that method in subclasses which cannot create engines
	virtual void do_parse_engine_from_config( const config &cfg, std::back_insert_iterator<std::vector< engine_ptr > > b );


	//do not override that method in subclasses which cannot create stages
	virtual void do_parse_stage_from_config( ai_context &context, const config &cfg, std::back_insert_iterator<std::vector< stage_ptr > > b );

	//do not override that method in subclasses which cannot evaluate formulas
	virtual std::string evaluate(const std::string& str);


	virtual const std::string& get_name() const;


	readonly_context& get_readonly_context();

	/**
	 * set ai context (which is not available during early initialization)
	 */
	virtual void set_ai_context(ai_context *context);


	/**
	 * serialize
	 */
	virtual config to_config() const;


	virtual const std::string& get_id() const;


	virtual const std::string& get_engine() const;

protected:
	readonly_context &ai_;

	/** name of the engine which has created this engine*/
	std::string engine_;
	std::string id_;
	std::string name_;
};


class engine_factory;

class engine_factory{
public:
	typedef boost::shared_ptr< engine_factory > factory_ptr;
	typedef std::map<std::string, factory_ptr> factory_map;
	typedef std::pair<const std::string, factory_ptr> factory_map_pair;

	static factory_map& get_list() {
		static factory_map *engine_factories;
		if (engine_factories==NULL) {
			engine_factories = new factory_map;
		}
		return *engine_factories;
	}

	virtual engine_ptr get_new_instance( readonly_context &ai, const config &cfg ) = 0;
	virtual engine_ptr get_new_instance( readonly_context &ai, const std::string& name ) = 0;

	engine_factory( const std::string &name )
	{
		factory_ptr ptr_to_this(this);
		get_list().insert(make_pair(name,ptr_to_this));
	}

	virtual ~engine_factory() {}
};


template<class ENGINE>
class register_engine_factory : public engine_factory {
public:
	register_engine_factory( const std::string &name )
		: engine_factory( name )
	{
	}

	virtual engine_ptr get_new_instance( readonly_context &ai, const config &cfg ){
		return engine_ptr(new ENGINE(ai,cfg));
	}

	virtual engine_ptr get_new_instance( readonly_context &ai, const std::string& name ){
		config cfg;
		cfg["name"] = name;
		cfg["engine"] = "cpp";
		return engine_ptr(new ENGINE(ai,cfg));
	}
};

} //end of namespace ai

#endif

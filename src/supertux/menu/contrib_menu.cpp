//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "supertux/menu/contrib_menu.hpp"

#include <physfs.h>
#include <sstream>

#include "gui/menu_manager.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/menu/contrib_levelset_menu.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/title_screen.hpp"
#include "supertux/world.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"

ContribMenu::ContribMenu() :
  m_contrib_worlds()
{
  /** Generating contrib levels list by making use of Level Subset  */
  std::vector<std::string> level_worlds;
  char** files = PHYSFS_enumerateFiles("levels/");
  for(const char* const* filename = files; *filename != 0; ++filename) {
    std::string filepath = std::string("levels/") + *filename;
    if(PHYSFS_isDirectory(filepath.c_str()))
      level_worlds.push_back(filepath);
  }
  PHYSFS_freeList(files);

  add_label(_("Contrib Levels"));
  add_hl();

  int i = 0;
  for (std::vector<std::string>::const_iterator it = level_worlds.begin(); it != level_worlds.end(); ++it)
  {
    try
    {
      std::unique_ptr<World> world = World::load(*it);

      if (!world->hide_from_contribs())
      {
        Savegame savegame(world->get_savegame_filename());
        savegame.load();

        if (world->is_levelset())
        {
          int level_count = 0;
          int solved_count = 0;

          const auto& state = savegame.get_levelset_state(world->get_basedir());
          for(const auto& level_state : state.level_states)
          {
            if (level_state.solved)
            {
              solved_count += 1;
            }
            level_count += 1;
          }

          std::ostringstream title;
          title << "[" << world->get_title() << "]";
          if (level_count == 0)
          {
            title << " *NEW*";
          }
          else
          {
            title << " (" << solved_count << "/" << level_count << ")";
          }
          add_entry(i++, title.str());
          m_contrib_worlds.push_back(std::move(world));
        }
        else if (world->is_worldmap())
        {
          int level_count = 0;
          int solved_count = 0;

          const auto& state = savegame.get_worldmap_state(world->get_worldmap_filename());
          for(const auto& level_state : state.level_states)
          {
            if (level_state.solved)
            {
              solved_count += 1;
            }
            level_count += 1;
          }

          std::ostringstream title;
          title << world->get_title();
          if (level_count == 0)
          {
            title << " *NEW*";
          }
          else
          {
            title << " (" << solved_count << "/" << level_count << ")";
          }
          add_entry(i++, title.str());
          m_contrib_worlds.push_back(std::move(world));
        }
        else
        {
          log_warning << "unknown World type" << std::endl;
        }
      }
    }
    catch(std::exception& e)
    {
      log_info << "Couldn't parse levelset info for '" << *it << "': " << e.what() << std::endl;
    }
  }

  add_hl();
  add_back(_("Back"));
}

ContribMenu::~ContribMenu()
{
}

void
ContribMenu::check_menu()
{
  int index = check();
  if (index != -1)
  {
    // reload the World so that we have something that we can safely
    // std::move() around without wreaking the ContribMenu
    std::unique_ptr<World> world = World::load(m_contrib_worlds[index]->get_basedir());
    if (!world->is_levelset())
    {
      GameManager::current()->start_game(std::move(world));
    }
    else
    {
      MenuManager::instance().push_menu(std::unique_ptr<Menu>(new ContribLevelsetMenu(std::move(world))));
    }
  }
}

/* EOF */

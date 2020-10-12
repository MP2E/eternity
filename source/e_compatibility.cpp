//
// Copyright (C) 2020 James Haley, Max Waine, Ioan Chera et al.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/
//
// Additional terms and conditions compatible with the GPLv3 apply. See the
// file COPYING-EE for details.
//
//----------------------------------------------------------------------------
//
// Purpose: EDF compatibility WAD support
// Authors: Ioan Chera
//

#include "z_zone.h"

#include "Confuse/confuse.h"
#include "c_io.h"
#include "doomstat.h"
#include "e_compatibility.h"
#include "m_collection.h"
#include "metaapi.h"

#define ITEM_COMPATIBILITY_HASHES "hashes"
#define ITEM_COMPATIBILITY_ENABLE "on"
#define ITEM_COMPATIBILITY_DISABLE "off"


// The EDF-set table
static MetaTable ontable;
static MetaTable offtable;

//
// The fields
//
cfg_opt_t edf_compatibility_opts[] =
{
   CFG_STR(ITEM_COMPATIBILITY_HASHES, 0, CFGF_LIST),
   CFG_STR(ITEM_COMPATIBILITY_ENABLE, 0, CFGF_LIST),
   CFG_STR(ITEM_COMPATIBILITY_DISABLE, 0, CFGF_LIST),

   CFG_END()
};

//
// Processes a compatibility item
//
static void E_processCompatibility(cfg_t *cfg, cfg_t* compatibility)
{
   unsigned hashCount = cfg_size(compatibility, ITEM_COMPATIBILITY_HASHES);
   unsigned onsettingCount = cfg_size(compatibility, ITEM_COMPATIBILITY_ENABLE);
   unsigned offsettingCount = cfg_size(compatibility, ITEM_COMPATIBILITY_DISABLE);

   if(!hashCount || (!onsettingCount && !offsettingCount))
      return;

   for(unsigned hashIndex = 0; hashIndex < hashCount; ++hashIndex)
   {
      const char *hash = cfg_getnstr(compatibility, ITEM_COMPATIBILITY_HASHES, hashIndex);
      for(unsigned settingIndex = 0; settingIndex < onsettingCount; ++settingIndex)
      {
         const char *setting = cfg_getnstr(compatibility, ITEM_COMPATIBILITY_ENABLE, settingIndex);

         bool found = false;
         MetaString *metaSetting = nullptr;
         while((metaSetting = ontable.getNextKeyAndTypeEx(metaSetting, hash)))
         {
            if(!strcasecmp(metaSetting->getValue(), setting))
            {
               found = true;
               break;
            }
         }

         if(!found)
            ontable.addString(hash, setting);
      }
      for(unsigned settingIndex = 0; settingIndex < offsettingCount; ++settingIndex)
      {
         const char *setting = cfg_getnstr(compatibility, ITEM_COMPATIBILITY_DISABLE, settingIndex);

         bool found = false;
         MetaString *metaSetting = nullptr;
         while((metaSetting = offtable.getNextKeyAndTypeEx(metaSetting, hash)))
         {
            if(!strcasecmp(metaSetting->getValue(), setting))
            {
               found = true;
               break;
            }
         }

         if(!found)
            offtable.addString(hash, setting);
      }
   }
}

//
// Process WAD compatibility
//
void E_ProcessCompatibilities(cfg_t* cfg)
{
   unsigned count = cfg_size(cfg, EDF_SEC_COMPATIBILITY);
   for(unsigned i = 0; i < count; ++i)
   {
      cfg_t* compatibility = cfg_getnsec(cfg, EDF_SEC_COMPATIBILITY, i);
      E_processCompatibility(cfg, compatibility);
   }
}

//
// Restore any pushed compatibilities
//
void E_RestoreCompatibilities()
{
   memset(level_compat_comp, 0, sizeof(level_compat_comp));
   memset(level_compat_compactive, 0, sizeof(level_compat_compactive));
}

//
// Look into comp_strings
//
static void E_setItem(const char *name, bool enable)
{
   extern const char *comp_strings[];
   // Look in the list of comp strings
   if(strncasecmp(name, "comp_", 5) != 0)
      return;

   for(int i = 0; i < COMP_NUM_USED; ++i)
   {
      if(!strcasecmp(name + 5, comp_strings[i]))
      {
         level_compat_compactive[i] = true;
         level_compat_comp[i] = enable;
         return;
      }
   }
   if(!strcasecmp(name, "comp_jump"))  // the only badly named entry
   {
      level_compat_compactive[comp_jump] = true;
      level_compat_comp[comp_jump] = enable;
   }
}

//
// Apply compatibility given a digest
//
void E_ApplyCompatibility(const char *digest)
{
   E_RestoreCompatibilities();

   MetaString *metaSetting = nullptr;
   while((metaSetting = ontable.getNextKeyAndTypeEx(metaSetting, digest)))
      E_setItem(metaSetting->getValue(), true);
   metaSetting = nullptr;
   while((metaSetting = offtable.getNextKeyAndTypeEx(metaSetting, digest)))
      E_setItem(metaSetting->getValue(), false);
}

// EOF

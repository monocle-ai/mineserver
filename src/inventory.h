/*
   Copyright (c) 2011, The Mineserver Project
   All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
  * Neither the name of the The Mineserver Project nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _INVENTORY_H_
#define _INVENTORY_H_

#include <stdint.h>
#include <vector>
#include <memory>
#include "nbt.h"

class User;

class Item
{
public:
  void setType(int16_t type);
  void setCount(int8_t count);
  void setHealth(int16_t healt);
  void setData(NBT_Value* _data);

  static bool isEnchantable(int16_t type)
  {
    return (256 <= type && type <= 259) ||
           (267 <= type && type <= 279) ||
           (283 <= type && type <= 286) ||
           (290 <= type && type <= 294) ||
           (298 <= type && type <= 317) ||
            type == 261 || type == 359 ||
            type == 346;
  }

  inline int16_t getType()   const { return type;   }
  inline int8_t  getCount()  const { return count;  }
  inline int16_t getHealth() const { return health; }
  inline NBT_Value* getData() const { return data; }

  int16_t itemHealth(int type);
  void decCount(int c = 1);
  void incHealth(int c = 1);
  void sendUpdate();
  bool ready;

private:
  User* player;
  int slot;
  int16_t type;
  int8_t count;
  int16_t health;
  NBT_Value *data;

public:

  ~Item()
  {
    if (data) delete data;
  }

  Item(User* player = NULL, int slot = -1)
    :
    ready(false),
    player(player),
    slot(slot),
    type(-1),
    count(0),
    health(0),
    data(nullptr)
  {
  }

  Item(int16_t _type, int8_t _count, int16_t _health, User* player = NULL)
    :
    ready(false),
    player(player),
    slot(-1),
    type(_type),
    count(_count),
    health(_health),
    data(nullptr)
  {
  }
};

typedef std::shared_ptr<Item> ItemPtr;

struct OpenInventory
{
  OpenInventory(): recordAction(false){};
  int8_t type;
  int32_t x;
  int32_t y;
  int32_t z;
  Item workbench[10];
  std::vector<User*> users;
  std::vector<int16_t> slotActions;
  bool recordAction;
};

typedef std::shared_ptr<OpenInventory> OpenInvPtr;

enum { WINDOW_CURSOR = -1, WINDOW_PLAYER = 0, WINDOW_CRAFTING_TABLE, WINDOW_CHEST, WINDOW_LARGE_CHEST, WINDOW_FURNACE, WINDOW_BREWING_STAND };

enum { MAX_SLOT_PLAYER = 44, MAX_SLOT_CHEST = 62, MAX_SLOT_LARGE_CHEST = 89,
       MAX_SLOT_CRAFTING_TABLE = 45, MAX_SLOT_FURNACE=38, MAX_SLOT_BREWING_STAND = 39};

// clickWindow special slot values
enum { SLOT_SPECIAL };

// clickWindow button values
enum { INVENTORY_BUTTON_LEFT = 0, INVENTORY_BUTTON_RIGHT,
       INVENTORY_BUTTON_ADD_SLOTS_LEFT = 1,
       INVENTORY_BUTTON_END_DRAG_LEFT,
       INVENTORY_BUTTON_START_DRAG_RIGHT = 4, INVENTORY_BUTTON_ADD_SLOTS_RIGHT = 5,
       INVENTORY_BUTTON_END_DRAG_RIGHT };

// clickWindow mode values
enum { INVENTORY_MODE_NORMAL, INVENTORY_MODE_SHIFT, INVENTORY_MODE_NUMBER, INVENTORY_MODE_MIDDLECLICK,
       INVENTORY_MODE_DROP_ITEM, INVENTORY_MODE_DRAG, INVENTORY_MODE_DOUBLECLICK };

#define INVENTORYTYPE_CHEST "minecraft:chest"
#define INVENTORYTYPE_CRAFTING_TABLE "minecraft:crafting_table"
#define INVENTORYTYPE_FURNACE "minecraft:furnace"
#define INVENTORYTYPE_BREWING_STAND "minecraft:brewing_stand"
#define INVENTORYTYPE_HORSE "EntityHorse"

class User;

class Inventory
{
public:

  struct Recipe
  {
    Recipe() : width(0), height(0), slots() {}

    int8_t width;
    int8_t height;
    std::vector<ItemPtr> slots;
    Item output;
  };

  typedef std::shared_ptr<Recipe> RecipePtr;

  bool addRecipe(int width, int height, std::vector<ItemPtr> inputrecipe,
                 int outputCount, int16_t outputType, int16_t outputHealth);
  bool readRecipe(const std::string& recipeFile);

  Inventory(const std::string& path, const std::string& suffix, const std::string& cfg);

  // Recipes
  std::vector<RecipePtr> recipes;

  // Open chest/workbench/furnace inventories
  std::vector<OpenInvPtr> openWorkbenches;
  std::vector<OpenInvPtr> openChests;
  std::vector<OpenInvPtr> openFurnaces;

  bool canBeArmour(int slot, int type);
  bool onupdateinventory(User* user, int8_t windowID);


  bool windowOpen(User* user, int8_t type, int32_t x, int32_t y, int32_t z);
  bool windowClose(User* user, int8_t type, int32_t x, int32_t y, int32_t z);

  bool windowClick(User* user, int8_t windowID, int16_t slot, int8_t button, int16_t actionNumber, int16_t itemID, int8_t itemCount, int16_t itemUses, int8_t mode = 0);

  bool windowClose(User* user, int8_t windowID);

  //Check inventory for space
  bool isSpace(User* user, int16_t itemID, char count);

  //Add items to inventory (pickups)
  bool addItems(User* user, int16_t itemID, int16_t count, int16_t health);

  bool updateInventory(User* user, int8_t windowID);
  bool doCraft(Item* slots, int8_t width, int8_t height);

  bool setSlot(User* user, int8_t windowID, int16_t slot, Item* item);

  int16_t itemHealth(int16_t itemID, int8_t block, bool& rightUse);

  void getEnabledRecipes(std::vector<std::string>& receiptFiles, const std::string& cfg);

};

#endif

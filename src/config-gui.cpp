/**
 * @file config-gui.cpp
 * @date 1-Oct-2023
 */

#include "orx.h"

#define orxIMGUI_IMPL
#include "orxImGui.h"
#undef orxIMGUI_IMPL

#ifdef __orxMSVC__

/* Requesting high performance dedicated GPU on hybrid laptops */
__declspec(dllexport) unsigned long NvOptimusEnablement = 1;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

#endif // __orxMSVC__

void orxConfig_DrawValue(const orxSTRING key)
{
  if (orxConfig_IsList(key))
  {
    // Show all values from the list
    if (ImGui::BeginCombo("", orxConfig_GetListString(key, 0)))
    {
      for (int i = 1; i < orxConfig_GetListCount(key); i++)
      {
        ImGui::Text("%s", orxConfig_GetListString(key, i));
      }
      ImGui::EndCombo();
    }
  }
  else
  {
    // Show text representation of a single value
    ImGui::Text("%s", orxConfig_GetString(key));
  }
}

void orxConfig_DrawSection(const orxSTRING section)
{
  // Push the selected config section to make it active
  orxConfig_PushSection(section);

  // Create a table with one row per key/value pair in the section
  auto keys = orxConfig_GetKeyCount();
  if (ImGui::BeginTable(section, 2))
  {
    for (int i = 0; i < keys; i++)
    {
      ImGui::TableNextRow();

      // Draw the key column
      ImGui::TableNextColumn();
      auto key = orxConfig_GetKey(i);
      ImGui::Text("%s", key);

      // Draw the value column
      ImGui::TableNextColumn();
      orxConfig_DrawValue(key);
    }
    ImGui::EndTable();
  }

  orxConfig_PopSection();
}

const orxSTRING orxConfig_GetGUISelectedSection()
{
  orxConfig_PushSection("GUISections");
  auto section = orxConfig_GetString("Selected");
  orxConfig_PopSection();
  return section;
}

void orxConfig_DrawSectionViewerWindow()
{
  // Begin a new window
  ImGui::Begin("Config section viewer");

  if (ImGui::BeginCombo("", orxConfig_GetGUISelectedSection()))
  {
    for (int i = 0; i < orxConfig_GetSectionCount(); i++)
    {
      auto section = orxConfig_GetSection(i);
      orxConfig_PushSection("GUISections");
      auto wasSelected = orxConfig_GetBool(section);
      auto selected = ImGui::Selectable(section, wasSelected);
      orxConfig_SetBool(section, selected);
      if (selected)
      {
        orxConfig_SetString("Selected", section);
      }
      orxConfig_PopSection();
    }
    ImGui::EndCombo();
  }

  // Draw selected config section's content
  auto section = orxConfig_GetGUISelectedSection();
  if (orxString_GetLength(section) > 0)
  {
    orxConfig_DrawSection(section);
  }

  // Let ImGui know the window is complete
  ImGui::End();
}

/** Update function, it has been registered to be called every tick of the core clock
 */
void orxFASTCALL Update(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
  // Show some config
  orxConfig_DrawSectionViewerWindow();

  // Should quit?
  if (orxInput_IsActive("Quit"))
  {
    // Send close event
    orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLOSE);
  }
}

/** Init function, it is called when all orx's modules have been initialized
 */
orxSTATUS orxFASTCALL Init()
{
  // Initialize Dear ImGui
  orxImGui_Init();

  // Register the Update function to the core clock
  orxClock_Register(orxClock_Get(orxCLOCK_KZ_CORE), Update, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_NORMAL);

  // Create the scene
  orxObject_CreateFromConfig("Scene");

  // Create the viewport
  orxViewport_CreateFromConfig("MainViewport");

  // Done!
  return orxSTATUS_SUCCESS;
}

/** Run function, it should not contain any game logic
 */
orxSTATUS orxFASTCALL Run()
{
  // Return orxSTATUS_FAILURE to instruct orx to quit
  return orxSTATUS_SUCCESS;
}

/** Exit function, it is called before exiting from orx
 */
void orxFASTCALL Exit()
{
  // Exit from Dear ImGui
  orxImGui_Exit();

  // Let orx clean all our mess automatically. :)
}

/** Bootstrap function, it is called before config is initialized, allowing for early resource storage definitions
 */
orxSTATUS orxFASTCALL Bootstrap()
{
  // Add config storage to find the initial config file
  orxResource_AddStorage(orxCONFIG_KZ_RESOURCE_GROUP, "../data/config", orxFALSE);

  // Return orxSTATUS_FAILURE to prevent orx from loading the default config file
  return orxSTATUS_SUCCESS;
}

/** Main function
 */
int main(int argc, char **argv)
{
  // Set the bootstrap function to provide at least one resource storage before loading any config files
  orxConfig_SetBootstrap(Bootstrap);

  // Execute our game
  orx_Execute(argc, argv, Init, Run, Exit);

  // Done!
  return EXIT_SUCCESS;
}

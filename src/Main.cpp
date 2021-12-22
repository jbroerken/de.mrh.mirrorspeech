/**
 *  Main.cpp
 *
 *  This file is part of the MRH project.
 *  See the AUTHORS file for Copyright information.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// C / C++
#include <cstdio>
#include <string>
#include <iostream>

// External
#include <libmrh/MRH_AppLoop.h>
#include <libmrhab.h>

// Project
#include "./Module/RepeatAfterMe/RepeatAfterMe.h"
#include "./Revision.h"

// Pre-defined
namespace
{
    libmrhab* p_Context = NULL;
    bool b_CloseApp = false;

    constexpr int i_CallbackThreadCount = 0;
}


// Prevent name wrangling for library header functions
#ifdef __cplusplus
extern "C"
{
#endif
    
    //*************************************************************************************
    // Init
    //*************************************************************************************

    int MRH_Init(const char* p_LaunchInput, int i_LaunchCommandID)
    {
        MRH_ModuleLogger& c_Logger = MRH_ModuleLogger::Singleton();
        c_Logger.Log("MRH_Init", "Initializing application (Version: " +
                                 std::string(REVISION_STRING) +
                                 ")",
                     "Main.cpp", __LINE__);
    
        try
        {
            p_Context = new libmrhab(std::make_unique<RepeatAfterMe>(),
                                     i_CallbackThreadCount);
            return 0;
        }
        catch (MRH_ABException& e)
        {
            c_Logger.Log("MRH_Init", "Failed to initialize app base library: " +
                                     e.what2(),
                         "Main.cpp", __LINE__);
            return -1;
        }
        catch (std::exception& e) // alloc and other stuff
        {
            c_Logger.Log("MRH_Init", "General exception: " +
                                     std::string(e.what()),
                         "Main.cpp", __LINE__);
            return -1;
        }
    }

    //*************************************************************************************
    // Recieve Event
    //*************************************************************************************

    void MRH_RecieveEvent(const MRH_Event* p_Event)
    {
        try
        {
            p_Context->AddJob(p_Event);
        }
        catch (MRH_ABException& e)
        {
            MRH_ModuleLogger::Singleton().Log("MRH_RecieveEvent", "Failed to add event job: " +
                                                                  e.what2(),
                                              "Main.cpp", __LINE__);
        }
    }

    //*************************************************************************************
    // Send Event
    //*************************************************************************************

    MRH_Event* MRH_SendEvent(void)
    {
        static bool b_UpdateModules = true;
    
        if (b_UpdateModules == true)
        {
            try
            {
                LIBMRHAB_UPDATE_RESULT b_Result = p_Context->Update();
            
                if (b_Result == LIBMRHAB_UPDATE_CLOSE_APP)
                {
                    b_CloseApp = true;
                }
            }
            catch (MRH_ABException& e)
            {
                MRH_ModuleLogger::Singleton().Log("MRH_SendEvent", "Module update failed: " +
                                                                   e.what2(),
                                                  "Main.cpp", __LINE__);
            
                // Stop sending immediatly to get to CanExit
                b_CloseApp = true;
                return NULL;
            }
        
            b_UpdateModules = false;
        }
    
        MRH_Event* p_Event = MRH_EventStorage::Singleton().GetEvent(true);
    
        if (p_Event == NULL)
        {
            b_UpdateModules = true;
        }
        
        return p_Event;
    }

    //*************************************************************************************
    // Exit
    //*************************************************************************************

    int MRH_CanExit(void)
    {
        return b_CloseApp == true ? 0 : -1;
    }

    void MRH_Exit(void)
    {
        if (p_Context != NULL)
        {
            delete p_Context;
        }
    }

#ifdef __cplusplus
}
#endif

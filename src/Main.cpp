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
//#include <libmrhpcdev.h>

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
            p_Context = new libmrhab(std::make_unique<RepeatAfterMe>(i_LaunchCommandID == 0 ? false : true),
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
    
    //*************************************************************************************
    // PC Dev
    //*************************************************************************************
    
#ifdef MRH_PC_DEV_LIB_IN_USE
    
    namespace
    {
        // @NOTE: Value runs from -2 to 0.
        //        2 SAY outputs for each input, ASK -> LISTEN -> REPEAT -> ASK -> ...
        //        Start at -1, first run only has 1 output before listen!
        int i_CanAnswer = -1;
    }
    
    MRH_Event* CallbackListen(const MRH_Event* p_Event)
    {
        static MRH_Uint32 u32_ID = 0;
        ++u32_ID;
        
        return MRH_L_STRING_S(MRH_L_STRING_S::END, u32_ID, 0, "Hello!").Build();
    }
    
    MRH_Event* CallbackSay(const MRH_Event* p_Event)
    {
        static MRH_SpeechString c_String(0);
        MRH_S_STRING_U c_Event(p_Event);
        bool b_End = c_Event.GetType() == MRH_S_STRING_U::END ? true : false;
        
        if (c_Event.GetID() == c_String.GetID())
        {
            c_String.Add(c_Event.GetString(),
                         c_Event.GetPart(),
                         b_End);
        }
        else
        {
            c_String.Reset(c_Event.GetString(),
                           c_Event.GetID(),
                           c_Event.GetPart(),
                           b_End);
        }
        
        if (c_String.GetState() == MRH_SpeechString::COMPLETE)
        {
            std::cout << c_String.GetString() << std::endl;
            c_String.Reset(0);
            
            ++i_CanAnswer;
        }
        
        return NULL;
    }
    
    MRH_Event* CallbackLaunch(const MRH_Event* p_Event)
    {
        MRH_A_LAUNCH_SOA_U c_Event(p_Event);
        return MRH_A_LAUNCH_SOA_S(c_Event.GetPackagePath(),
                                  c_Event.GetLaunchInput(),
                                  c_Event.GetLaunchCommandID()).Build();
    }
    
    MRH_Event* CallbackServiceAvail(const MRH_Event* p_Event)
    {
        switch (p_Event->u32_Type)
        {
            case MRH_EVENT_LISTEN_AVAIL_U:
                return MRH_L_AVAIL_S(true, 0, 0, 1).Build();
            case MRH_EVENT_SAY_AVAIL_U:
                return MRH_S_AVAIL_S(true, 0, 0, 1).Build();
                
            default:
                return NULL;
        }
    }
    
    int MRH_PCD_Setup(void)
    {
        // Set app environment
        MRH_PCD_EnvironmentType(MRH_PCDEV_ENV_APPLICATION);
        MRH_PCD_FSRoot("/Users/Jens/Documents/GitHub/MRH/App/de.mrh.mirrorspeech/pkg/FSRoot/");
        
        // Add event callbacks
        MRH_PCD_EventSending(10, 100);
        MRH_PCD_EventCallback(&CallbackListen, MRH_EVENT_LISTEN_STRING_S);
        MRH_PCD_EventCallback(&CallbackSay, MRH_EVENT_SAY_STRING_U);
        MRH_PCD_EventCallback(&CallbackServiceAvail, MRH_EVENT_LISTEN_AVAIL_U);
        MRH_PCD_EventCallback(&CallbackServiceAvail, MRH_EVENT_SAY_AVAIL_U);
        
        // Set permissions
        MRH_PCD_PermissionsListen(MRH_PCD_PERMISSION_LISTEN_LISTEN);
        MRH_PCD_PermissionsSay(MRH_PCD_PERMISSION_SAY_SAY);
        
        // Set app launch input
        MRH_PCD_AppLaunchParam("", 0, 0);
        
        // Add Functions
        MRH_PCD_FunctionAppInit(&MRH_Init);
        MRH_PCD_FunctionAppRecieveEvent(&MRH_RecieveEvent);
        MRH_PCD_FunctionSendEvent(&MRH_SendEvent);
        MRH_PCD_FunctionAppCanExit(&MRH_CanExit);
        MRH_PCD_FunctionExit(&MRH_Exit);
        
        return 0;
    }
    
    int MRH_PCD_Update(void)
    {
        if (i_CanAnswer == 0)
        {
            i_CanAnswer = -2;
            MRH_PCD_TriggerCallback(NULL, MRH_EVENT_LISTEN_STRING_S);
        }
        
        return 0;
    }
    
#endif

#ifdef __cplusplus
}
#endif

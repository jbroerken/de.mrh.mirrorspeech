/**
 *  RepeatAfterMe.cpp
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

// External
#include <libmrhvt/Output/MRH_OutputGenerator.h>

// Project
#include "./RepeatAfterMe.h"

// Pre-defined
#ifndef SPEECH_OUTPUT_DIR
    #define SPEECH_OUTPUT_DIR "Output"
#endif
#ifndef SPEECH_OUTPUT_FILE
    #define SPEECH_OUTPUT_FILE "WhatInput.mrhog"
#endif
#ifndef STATE_TIMEOUT_MS
    #define STATE_TIMEOUT_MS 30000
#endif


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

RepeatAfterMe::RepeatAfterMe(bool b_Endless) : MRH_Module("RepeatAfterMe"),
                                               MRH_ModuleTimer(STATE_TIMEOUT_MS),
                                               b_Endless(b_Endless),
                                               e_State(SERVICE_CHECK),
                                               i_ServiceAvail(0)
{
    // Prepare service available events
    MRH_EventStorage& c_Storage = MRH_EventStorage::Singleton();
    
    c_Storage.Add(MRH_L_AVAIL_U());
    c_Storage.Add(MRH_S_AVAIL_U());
}

RepeatAfterMe::~RepeatAfterMe() noexcept
{}

//*************************************************************************************
// Update
//*************************************************************************************

void RepeatAfterMe::HandleEvent(const MRH_EVBase* p_Event) noexcept
{
    // @NOTE: No locking required, only 1 callback thread in use
    
    switch (p_Event->GetType())
    {
        // Service
        case MRH_EVENT_LISTEN_AVAIL_S:
        case MRH_EVENT_SAY_AVAIL_S:
            if (event_cast<const MRH_EvAvailService*>(p_Event)->GetUsable() == true)
            {
                int i_Flag = p_Event->GetType() == MRH_EVENT_LISTEN_AVAIL_S ? 1 : 2;
                
                if ((i_ServiceAvail += (i_ServiceAvail & i_Flag ? 0 : i_Flag)) == 3)
                {
                    SetState(ASK_OUTPUT);
                }
            }
            else
            {
                i_ServiceAvail = -1;
            }
            break;
            
        // Input
        case MRH_EVENT_LISTEN_STRING_S:
            if (e_State == LISTEN_INPUT)
            {
                try
                {
                    AddInput(event_cast<const MRH_L_STRING_S*>(p_Event));
                    
                    if (GetInputFinished() == true)
                    {
                        SetState(REPEAT_OUTPUT);
                    }
                }
                catch (MRH_ModuleException& e)
                {
                    MRH_ModuleLogger::Singleton().Log("RepeatAfterMe", e.what(),
                                                      "RepeatAfterMe.cpp", __LINE__);
                }
            }
            break;
            
        // Output
        case MRH_EVENT_SAY_STRING_S:
            if (OutputPerformed(event_cast<const MRH_S_STRING_S*>(p_Event)) == true)
            {
                if (e_State == ASK_PERFORMED)
                {
                    SetState(LISTEN_INPUT);
                }
                else if (e_State == REPEAT_PERFORMED)
                {
                    SetState(b_Endless == true ? LISTEN_INPUT : CLOSE_APP);
                }
            }
            break;
            
        default:
            break;
    }
}

MRH_Module::Result RepeatAfterMe::Update()
{
    // Check timeout
    if (GetTimerFinished() == true)
    {
        throw MRH_ModuleException("RepeatAfterMe",
                                  "Timeout for state " + std::to_string(e_State) + "!");
    }
    else if (i_ServiceAvail < 0)
    {
        throw MRH_ModuleException("RepeatAfterMe",
                                  "Service(s) unavailable!");
    }
    
    try
    {
        switch (e_State)
        {
            // Output
            case ASK_OUTPUT:
                SendOutput(GetOutputStringID() + 1,
                           MRH_OutputGenerator(SPEECH_OUTPUT_DIR, SPEECH_OUTPUT_FILE).Generate());
                SetState(ASK_PERFORMED);
                break;
            case REPEAT_OUTPUT:
                SendOutput(GetOutputStringID() + 1,
                           BuildInput());
                SetState(REPEAT_PERFORMED);
                break;
                
            // App End
            case CLOSE_APP:
                return MRH_Module::FINISHED_POP;
                
            // Other stuff is handled in the HandleEvent() function
            default:
                break;
        }
        
        return MRH_Module::IN_PROGRESS;
    }
    catch (MRH_VTException& e)
    {
        throw MRH_ModuleException("RepeatAfterMe",
                                  "MRH_VTException: " + e.what2());
    }
    catch (MRH_ABException& e)
    {
        throw MRH_ModuleException("RepeatAfterMe",
                                  "MRH_ABException: " + e.what2());
    }
    catch (MRH_ModuleException& e)
    {
        throw MRH_ModuleException("RepeatAfterMe",
                                  "MRH_ModuleException: " + e.what2());
    }
}

std::shared_ptr<MRH_Module> RepeatAfterMe::NextModule()
{
    throw MRH_ModuleException("RepeatAfterMe",
                              "No module to switch to!");
}

//*************************************************************************************
// Getters
//*************************************************************************************

bool RepeatAfterMe::CanHandleEvent(MRH_Uint32 u32_Type) noexcept
{
    switch (u32_Type)
    {
        case MRH_EVENT_LISTEN_AVAIL_S:
        case MRH_EVENT_LISTEN_STRING_S:
        case MRH_EVENT_SAY_AVAIL_S:
        case MRH_EVENT_SAY_STRING_S:
            return true;
            
        default:
            return false;
    }
}

//*************************************************************************************
// Setters
//*************************************************************************************

void RepeatAfterMe::SetState(State e_State) noexcept
{
    if (this->e_State != e_State)
    {
        ResetTimer(STATE_TIMEOUT_MS);
        this->e_State = e_State;
    }
}

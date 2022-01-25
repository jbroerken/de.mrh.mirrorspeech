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
#include <libmrhevdata.h>
#include <libmrhvt/Output/MRH_OutputGenerator.h>

// Project
#include "./RepeatAfterMe.h"

// Pre-defined
#define speech_cast event_cast<const MRH_EvSpeechString*>
#define TIMEOUT_SERVICE_MS 15000
#define TIMEOUT_INPUT_MS 60000
#define TIMEOUT_OUTPUT_MS 30000
#define SPEECH_OUTPUT_DIR "Output"
#define SPEECH_OUTPUT_FILE "WhatInput.mrhog"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

RepeatAfterMe::RepeatAfterMe() : MRH_Module("RepeatAfterMe"),
                                 c_Input(0),
                                 u32_OutputID(0),
                                 i_Service(0)
{
    StateSet(CHECK_SERVICE);
}

RepeatAfterMe::~RepeatAfterMe() noexcept
{}

//*************************************************************************************
// Update
//*************************************************************************************

void RepeatAfterMe::HandleEvent(const MRH_Event* p_Event) noexcept
{
    switch (p_Event->u32_Type)
    {
        // Service
        case MRH_EVENT_LISTEN_AVAIL_S:
        case MRH_EVENT_SAY_AVAIL_S:
        {
            struct MRH_EvD_Base_ServiceAvail_S_t c_Data;
            
            if (MRH_EVD_ReadEvent(&c_Data, p_Event->u32_Type, p_Event) < 0)
            {
                break;
            }
            else if (c_Data.u8_Available != MRH_EVD_BASE_RESULT_SUCCESS)
            {
                break;
            }
            
            int i_Flag = p_Event->u32_Type == MRH_EVENT_LISTEN_AVAIL_S ? 1 : 2;
            
            if ((i_Service += (i_Service & i_Flag ? 0 : i_Flag)) == 3)
            {
                StateSet(ASK_OUTPUT);
            }
            break;
        }
            
        // Input
        case MRH_EVENT_LISTEN_STRING_S:
        {
           MRH_EvD_L_String_S c_Data;
            
            if (MRH_EVD_ReadEvent(&c_Data, p_Event->u32_Type, p_Event) < 0)
            {
                break;
            }
            
            if (c_Data.u32_ID != c_Input.GetID())
            {
                c_Input.Reset(c_Data.p_String,
                              c_Data.u32_ID,
                              c_Data.u32_Part,
                              c_Data.u8_Type == MRH_EVD_L_STRING_END ? true : false);
            }
            else
            {
                c_Input.Add(c_Data.p_String,
                            c_Data.u32_Part,
                            c_Data.u8_Type == MRH_EVD_L_STRING_END ? true : false);
            }
            
            if (c_Input.GetState() == MRH_SpeechString::COMPLETE)
            {
                StateSet(REPEAT_OUTPUT);
            }
            break;
        }
            
        // Output
        case MRH_EVENT_SAY_STRING_S:
        {
            MRH_EvD_S_String_S c_Data;
            
            if (MRH_EVD_ReadEvent(&c_Data, p_Event->u32_Type, p_Event) < 0)
            {
                break;
            }
            else if (c_Data.u32_ID != u32_OutputID)
            {
                break;
            }
            
            if (e_State == ASK_OUTPUT)
            {
                StateSet(LISTEN_INPUT);
            }
            else if (e_State == REPEAT_OUTPUT)
            {
                StateSet(CLOSE_APP);
            }
            break;
        }
            
        // Unk
        default: { break; }
    }
}

MRH_Module::Result RepeatAfterMe::Update()
{
    switch (e_State)
    {
        case CHECK_SERVICE:
            StateCheckService();
            break;
        case ASK_OUTPUT:
            StateAskOutput();
            break;
        case LISTEN_INPUT:
            break;
        case REPEAT_OUTPUT:
            StateRepeatOutput();
            break;
            
        default:
            return MRH_Module::FINISHED_POP;
            
    }
    
    return MRH_Module::IN_PROGRESS;
}

std::shared_ptr<MRH_Module> RepeatAfterMe::NextModule()
{
    throw MRH_ModuleException("RepeatAfterMe",
                              "No module to switch to!");
}

//*************************************************************************************
// State
//*************************************************************************************

void RepeatAfterMe::StateSet(State e_State) noexcept
{
    this->e_State = e_State;
    ResetTimer();
}

void RepeatAfterMe::StateCheckService() noexcept
{
    if (GetTimerSet() == false)
    {
        MRH_EventStorage::Singleton().Add(MRH_EVD_CreateEvent(MRH_EVENT_LISTEN_AVAIL_U, NULL, 0));
        MRH_EventStorage::Singleton().Add(MRH_EVD_CreateEvent(MRH_EVENT_SAY_AVAIL_U, NULL, 0));
        
        SetTimer(TIMEOUT_SERVICE_MS);
    }
    else if (i_Service == 3)
    {
        StateSet(ASK_OUTPUT);
        i_Service = 0;
    }
    else if (GetTimerFinished() == true)
    {
        MRH_ModuleLogger::Singleton().Log("RepeatAfterMe", "Check service timeout!",
                                          "RepeatAfterMe.cpp", __LINE__);
        StateSet(CLOSE_APP);
    }
}

void RepeatAfterMe::StateSendOutput(std::string const& s_String) noexcept
{
    MRH_EventStorage& c_Storage = MRH_EventStorage::Singleton();
    
    MRH_Event* p_Event = NULL;
    MRH_EvD_S_String_U c_Data;
    
    try
    {
        std::map<MRH_Uint32, std::string> m_Part(MRH_SpeechString::SplitString(s_String));
        u32_OutputID = (rand() % ((MRH_Uint32) - 1));
        
        memset((c_Data.p_String), '\0', MRH_EVD_S_STRING_BUFFER_MAX_TERMINATED);
        
        for (auto It = m_Part.begin(); It != m_Part.end(); ++It)
        {
            if (It == --(m_Part.end()))
            {
                memset((c_Data.p_String), '\0', MRH_EVD_S_STRING_BUFFER_MAX_TERMINATED);
                c_Data.u8_Type = MRH_EVD_L_STRING_END;
            }
            else
            {
                c_Data.u8_Type = MRH_EVD_L_STRING_UNFINISHED;
            }
            
            strcpy((c_Data.p_String), (It->second.c_str()));
            
            c_Data.u32_ID = u32_OutputID;
            c_Data.u32_Part = It->first;
            
            if (p_Event == NULL && (p_Event = MRH_EVD_CreateEvent(MRH_EVENT_SAY_STRING_U, NULL, 0)) == NULL)
            {
                continue;
            }
            else if (MRH_EVD_SetEvent(p_Event, MRH_EVENT_SAY_STRING_U, &c_Data) < 0)
            {
                continue;
            }
            
            c_Storage.Add(p_Event);
            p_Event = NULL;
        }
    }
    catch (std::exception& e)
    {
        MRH_ModuleLogger::Singleton().Log("RepeatAfterMe", "Failed to repeat output: " +
                                                           std::string(e.what()),
                                          "RepeatAfterMe.cpp", __LINE__);
    }
    
    if (p_Event != NULL)
    {
        MRH_EVD_DestroyEvent(p_Event);
    }
}

void RepeatAfterMe::StateAskOutput() noexcept
{
    if (GetTimerSet() == false)
    {
        try
        {
            StateSendOutput(MRH_OutputGenerator(SPEECH_OUTPUT_DIR, SPEECH_OUTPUT_FILE).Generate());
        }
        catch (std::exception& e)
        {
            MRH_ModuleLogger::Singleton().Log("RepeatAfterMe", "Failed to ask output: " +
                                                               std::string(e.what()),
                                              "RepeatAfterMe.cpp", __LINE__);
        }
        
        SetTimer(TIMEOUT_OUTPUT_MS);
    }
    else if (GetTimerFinished() == true)
    {
        MRH_ModuleLogger::Singleton().Log("RepeatAfterMe", "Ask repeat timeout!",
                                          "RepeatAfterMe.cpp", __LINE__);
        StateSet(CLOSE_APP);
    }
}

void RepeatAfterMe::StateRepeatOutput() noexcept
{
    if (GetTimerSet() == false)
    {
        StateSendOutput(c_Input.GetString());
        SetTimer(TIMEOUT_OUTPUT_MS);
    }
    else if (GetTimerFinished() == true)
    {
        StateSet(CLOSE_APP);
    }
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

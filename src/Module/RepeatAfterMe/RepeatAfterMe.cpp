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
#define speech_cast event_cast<const MRH_EvSpeechString*>
#define TIMEOUT_SERVICE_MS 5000
#define TIMEOUT_INPUT_MS 30000
#define TIMEOUT_OUTPUT_MS 15000
#define SPEECH_OUTPUT_DIR "Output"
#define SPEECH_OUTPUT_FILE "WhatInput.mrhog"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

RepeatAfterMe::RepeatAfterMe(bool b_Endless) : MRH_Module("RepeatAfterMe"),
                                               b_Endless(b_Endless),
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

void RepeatAfterMe::HandleEvent(const MRH_EVBase* p_Event) noexcept
{
    switch (p_Event->GetType())
    {
        // Service
        case MRH_EVENT_LISTEN_AVAIL_S:
        case MRH_EVENT_SAY_AVAIL_S:
            if (event_cast<const MRH_EvAvailService*>(p_Event)->GetUsable() == true)
            {
                int i_Flag = p_Event->GetType() == MRH_EVENT_LISTEN_AVAIL_S ? 1 : 2;
                
                if ((i_Service += (i_Service & i_Flag ? 0 : i_Flag)) == 3)
                {
                    StateSet(ASK_OUTPUT);
                }
            }
            break;
            
        // Input
        case MRH_EVENT_LISTEN_STRING_S:
            if (speech_cast(p_Event)->GetID() != c_Input.GetID())
            {
                c_Input.Reset(speech_cast(p_Event)->GetString(),
                              speech_cast(p_Event)->GetID(),
                              speech_cast(p_Event)->GetPart(),
                              speech_cast(p_Event)->GetType() == MRH_EvSpeechString::END ? true : false);
            }
            else
            {
                c_Input.Add(speech_cast(p_Event)->GetString(),
                            speech_cast(p_Event)->GetPart(),
                            speech_cast(p_Event)->GetType() == MRH_EvSpeechString::END ? true : false);
            }
            
            if (c_Input.GetState() == MRH_SpeechString::COMPLETE)
            {
                StateSet(REPEAT_OUTPUT);
            }
            break;
            
        // Output
        case MRH_EVENT_SAY_STRING_S:
            if (speech_cast(p_Event)->GetID() == u32_OutputID)
            {
                if (e_State == ASK_OUTPUT)
                {
                    StateSet(LISTEN_INPUT);
                }
                else if (e_State == REPEAT_OUTPUT)
                {
                    StateSet(CLOSE_APP);
                }
            }
            break;
            
        default:
            break;
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
    if (e_State == CLOSE_APP && b_Endless == true)
    {
        this->e_State = CHECK_SERVICE;
    }
    else
    {
        this->e_State = e_State;
    }
    
    ResetTimer();
}

void RepeatAfterMe::StateCheckService() noexcept
{
    if (GetTimerSet() == false)
    {
        MRH_EventStorage::Singleton().Add(MRH_L_AVAIL_U());
        MRH_EventStorage::Singleton().Add(MRH_S_AVAIL_U());
        
        SetTimer(TIMEOUT_SERVICE_MS);
    }
    else if (i_Service == 3)
    {
        StateSet(ASK_OUTPUT);
        i_Service = 0;
    }
    else if (GetTimerFinished() == true)
    {
        StateSet(CLOSE_APP);
    }
}

void RepeatAfterMe::StateSendOutput(std::string const& s_String) noexcept
{
    MRH_EventStorage& c_Storage = MRH_EventStorage::Singleton();
    
    try
    {
        std::map<MRH_Uint32, std::string> m_Part(MRH_SpeechString::SplitString(s_String));
        ++u32_OutputID;
        
        for (auto It = m_Part.begin(); It != m_Part.end(); ++It)
        {
            c_Storage.Add(MRH_S_STRING_U((It == --(m_Part.end())) ? MRH_S_STRING_U::END : MRH_S_STRING_U::UNFINISHED,
                                         u32_OutputID,
                                         It->first,
                                         It->second));
        }
    }
    catch (std::exception& e)
    {
        MRH_ModuleLogger::Singleton().Log("RepeatAfterMe", "Failed to repeat output: " +
                                                           std::string(e.what()),
                                          "RepeatAfterMe.cpp", __LINE__);
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

/**
 *  SpeechInput.cpp
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

// Project
#include "./SpeechInput.h"

// Pre-defined
#ifndef MODULE_SPEECH_INPUT_TIMEOUT_MS
    #define MODULE_SPEECH_INPUT_TIMEOUT_MS 30000
#endif


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

SpeechInput::SpeechInput(std::string& s_Input) noexcept : MRH_Module("SpeechInput"),
                                                          c_Timer(),
                                                          c_SpeechString(0),
                                                          s_Input(s_Input)
{
    c_Timer.SetTimer(MODULE_SPEECH_INPUT_TIMEOUT_MS);
}

SpeechInput::~SpeechInput() noexcept
{}

//*************************************************************************************
// Update
//*************************************************************************************

void SpeechInput::HandleEvent(const MRH_Event* p_Event) noexcept
{
    // @NOTE: CanHandleEvent() allows skipping event type check!
    MRH_EvD_L_String_S c_String;
    
    if (MRH_EVD_ReadEvent(&c_String, p_Event->u32_Type, p_Event) < 0)
    {
        MRH_ModuleLogger::Singleton().Log("SpeechInput", "Failed to read service avail event!",
                                          "SpeechInput.cpp", __LINE__);
        return;
    }
    
    if (c_SpeechString.GetID() != c_String.u32_ID)
    {
        c_SpeechString.Reset(c_String.p_String,
                             c_String.u32_ID,
                             c_String.u32_Part,
                             c_String.u8_Type == MRH_EVD_L_STRING_END ? true : false);
    }
    else
    {
        c_SpeechString.Add(c_String.p_String,
                           c_String.u32_Part,
                           c_String.u8_Type == MRH_EVD_L_STRING_END ? true : false);
    }
}

MRH_Module::Result SpeechInput::Update()
{
    if (c_SpeechString.GetState() == MRH_SpeechString::COMPLETE || c_Timer.GetTimerFinished() == true)
    {
        s_Input = c_SpeechString.GetString();
        return MRH_Module::FINISHED_POP;
    }
    
    return MRH_Module::IN_PROGRESS;
}

std::shared_ptr<MRH_Module> SpeechInput::NextModule()
{
    throw MRH_ModuleException("SpeechInput",
                              "No module to switch to!");
}

//*************************************************************************************
// Getters
//*************************************************************************************

bool SpeechInput::CanHandleEvent(MRH_Uint32 u32_Type) noexcept
{
    switch (u32_Type)
    {
        case MRH_EVENT_LISTEN_STRING_S:
            return true;
            
        default:
            return false;
    }
}

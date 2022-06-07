/**
 *  Copyright (C) 2021 - 2022 The MRH Project Authors.
 * 
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

// C / C++
#include <cstring>

// External

// Project
#include "./SpeechInput.h"

// Pre-defined
#ifndef SPEECH_INPUT_TIMEOUT_MS
    #define SPEECH_INPUT_TIMEOUT_MS 30000
#endif


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

SpeechInput::SpeechInput(std::string& s_Input) noexcept : MRH_Module("SpeechInput"),
                                                          c_Timer(SPEECH_INPUT_TIMEOUT_MS),
                                                          s_Input(s_Input)
{
    this->s_Input = "";
}

SpeechInput::~SpeechInput() noexcept
{}

//*************************************************************************************
// Update
//*************************************************************************************

void SpeechInput::HandleEvent(const MRH_Event* p_Event) noexcept
{
    MRH_EvD_L_String_S c_String;
    
    if (MRH_EVD_ReadEvent(&c_String, p_Event->u32_Type, p_Event) < 0)
    {
        MRH_ModuleLogger::Singleton().Log("SpeechInput", "Failed to read listen string event!",
                                          "SpeechInput.cpp", __LINE__);
        return;
    }
    
    if (strnlen(c_String.p_String, MRH_EVD_L_STRING_BUFFER_MAX_TERMINATED) > 0)
    {
        s_Input = c_String.p_String;
    }
}

MRH_Module::Result SpeechInput::Update()
{
    if (c_Timer.GetTimerFinished() == true || s_Input.size() > 0)
    {
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

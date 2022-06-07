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
#include "./SpeechOutput.h"

// Pre-defined
#ifndef SPEECH_OUTPUT_TIMEOUT_MS
    #define SPEECH_OUTPUT_TIMEOUT_MS 60000
#endif


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

SpeechOutput::SpeechOutput(std::string s_Output) : MRH_Module("SpeechOutput"),
                                                   c_Timer(SPEECH_OUTPUT_TIMEOUT_MS),
                                                   u32_SentOutputID((rand() % ((MRH_Uint32) - 1)) + 1),
                                                   u32_ReceivedOutputID(0)
{
    MRH_ModuleLogger::Singleton().Log("SpeechOutput", "Sending output: " +
                                                      s_Output +
                                                      " (ID: " +
                                                      std::to_string(u32_SentOutputID) +
                                                      ")",
                                      "SpeechOutput.cpp", __LINE__);
    // Setup event data
    MRH_EvD_S_String_U c_Data;
    
    memset((c_Data.p_String), '\0', MRH_EVD_S_STRING_BUFFER_MAX_TERMINATED);
    strncpy(c_Data.p_String, s_Output.c_str(), MRH_EVD_S_STRING_BUFFER_MAX);
    c_Data.u32_ID = u32_SentOutputID;
    
    // Create event
    MRH_Event* p_Event = MRH_EVD_CreateSetEvent(MRH_EVENT_SAY_STRING_U, &c_Data);
    
    if (p_Event == NULL)
    {
        throw MRH_ModuleException("SpeechOutput", 
                                  "Failed to create output event!");
    }
    
    // Attempt to add to out storage
    try
    {
        MRH_EventStorage::Singleton().Add(p_Event);
    }
    catch (MRH_ABException& e)
    {
        MRH_EVD_DestroyEvent(p_Event);
        throw MRH_ModuleException("SpeechOutput", 
                                  "Failed to send output: " + e.what2());
    }
}

SpeechOutput::~SpeechOutput() noexcept
{}

//*************************************************************************************
// Update
//*************************************************************************************

void SpeechOutput::HandleEvent(const MRH_Event* p_Event) noexcept
{
    // @NOTE: CanHandleEvent() allows skipping event type check!
    MRH_EvD_S_String_S c_String;
    
    if (MRH_EVD_ReadEvent(&c_String, p_Event->u32_Type, p_Event) < 0)
    {
        MRH_ModuleLogger::Singleton().Log("SpeechOutput", "Failed to read string event!",
                                          "SpeechOutput.cpp", __LINE__);
    }
    else
    {
        MRH_ModuleLogger::Singleton().Log("SpeechOutput", "Received output performed: " +
                                                          std::to_string(c_String.u32_ID),
                                          "SpeechOutput.cpp", __LINE__);
        
        u32_ReceivedOutputID = c_String.u32_ID;
    }
}

MRH_Module::Result SpeechOutput::Update()
{
    if (u32_SentOutputID == u32_ReceivedOutputID || c_Timer.GetTimerFinished() == true)
    {
        return MRH_Module::FINISHED_POP;
    }
    
    return MRH_Module::IN_PROGRESS;
}

std::shared_ptr<MRH_Module> SpeechOutput::NextModule()
{
    throw MRH_ModuleException("SpeechOutput",
                              "No module to switch to!");
}

//*************************************************************************************
// Getters
//*************************************************************************************

bool SpeechOutput::CanHandleEvent(MRH_Uint32 u32_Type) noexcept
{
    switch (u32_Type)
    {
        case MRH_EVENT_SAY_STRING_S:
            return true;
            
        default:
            return false;
    }
}

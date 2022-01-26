/**
 *  SpeechOutput.cpp
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
#include <libmrhvt.h>

// Project
#include "./SpeechOutput.h"

// Pre-defined
#ifndef MODULE_SPEECH_OUTPUT_TIMEOUT_MS
    #define MODULE_SPEECH_OUTPUT_TIMEOUT_MS 60000
#endif


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

SpeechOutput::SpeechOutput(std::string const& s_String) noexcept : MRH_Module("SpeechOutput"),
                                                                   c_Timer(),
                                                                   u32_SentOutputID((rand() % ((MRH_Uint32) - 1)) + 1),
                                                                   u32_RecievedOutputID(0)
{
    try
    {
        SendOutput(s_String);
    }
    catch (MRH_ModuleException& e)
    {
        MRH_ModuleLogger::Singleton().Log(e.module(), e.what(),
                                          "SpeechOutput.cpp", __LINE__);
    }
    
    c_Timer.SetTimer(MODULE_SPEECH_OUTPUT_TIMEOUT_MS);
}

SpeechOutput::SpeechOutput(std::string const& s_DirPath,
                           std::string const& s_FileName) noexcept : MRH_Module("SpeechOutput"),
                                                                     c_Timer(),
                                                                     u32_SentOutputID((rand() % ((MRH_Uint32) - 1)) + 1),
                                                                     u32_RecievedOutputID(0)
{
    try
    {
        SendOutput(MRH_OutputGenerator(s_DirPath, s_FileName).Generate());
    }
    catch (MRH_VTException& e)
    {
        MRH_ModuleLogger::Singleton().Log("SpeechOutput", e.what(),
                                          "SpeechOutput.cpp", __LINE__);
    }
    catch (MRH_ModuleException& e)
    {
        MRH_ModuleLogger::Singleton().Log(e.module(), e.what(),
                                          "SpeechOutput.cpp", __LINE__);
    }
    
    c_Timer.SetTimer(MODULE_SPEECH_OUTPUT_TIMEOUT_MS);
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
        u32_RecievedOutputID = c_String.u32_ID;
    }
}

MRH_Module::Result SpeechOutput::Update()
{
    if (u32_SentOutputID == u32_RecievedOutputID || c_Timer.GetTimerFinished() == true)
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
// Output
//*************************************************************************************

void SpeechOutput::SendOutput(std::string const& s_String)
{
    MRH_EventStorage& c_Storage = MRH_EventStorage::Singleton();
    
    MRH_Event* p_Event = NULL;
    MRH_EvD_S_String_U c_Data;
    
    try
    {
        std::map<MRH_Uint32, std::string> m_Part(MRH_SpeechString::SplitString(s_String));
        
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
            
            strncpy((c_Data.p_String), (It->second.c_str()), MRH_EVD_S_STRING_BUFFER_MAX);
            
            c_Data.u32_ID = u32_SentOutputID;
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
        MRH_ModuleLogger::Singleton().Log("SpeechOutput", "Failed to send output: " +
                                                           std::string(e.what()),
                                          "SpeechOutput.cpp", __LINE__);
    }
    
    if (p_Event != NULL)
    {
        MRH_EVD_DestroyEvent(p_Event);
    }
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

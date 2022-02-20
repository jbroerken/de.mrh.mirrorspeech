/**
 *  MirrorSpeech.cpp
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
#include <libmrhab/Module/Common/MRH_CommonModule.h>
#include <libmrhvt/Output/MRH_OutputGenerator.h>

// Project
#include "./MirrorSpeech.h"

// Pre-defined
#ifndef MIRROR_SPEECH_OUTPUT_DIR
    #define MIRROR_SPEECH_OUTPUT_DIR "Output"
#endif
#ifndef MIRROR_SPEECH_OUTPUT_FILE
    #define MIRROR_SPEECH_OUTPUT_FILE "WhatInput.mrhog"
#endif
#ifndef MIRROR_SPEECH_SERVICE_TIMEOUT_MS
    #define MIRROR_SPEECH_SERVICE_TIMEOUT_MS 10 * 1000
#endif
#ifndef MIRROR_SPEECH_SPEECH_TIMEOUT_MS
    #define MIRROR_SPEECH_SPEECH_TIMEOUT_MS 60 * 1000
#endif


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

MirrorSpeech::MirrorSpeech() noexcept : MRH_Module("MirrorSpeech"),
                                        e_State(START),
                                        s_Input(""),
                                        b_ServiceAvailable(false)
{}

MirrorSpeech::~MirrorSpeech() noexcept
{}

//*************************************************************************************
// Update
//*************************************************************************************

void MirrorSpeech::HandleEvent(const MRH_Event* p_Event) noexcept
{}

MRH_Module::Result MirrorSpeech::Update()
{
    switch (e_State)
    {
        case START:
            e_State = CHECK_LISTEN;
            return MRH_Module::FINISHED_APPEND;
            
        case CHECK_LISTEN:
        case CHECK_SAY:
            if (b_ServiceAvailable == false)
            {
                return MRH_Module::FINISHED_POP;
            }
            
            e_State = (e_State == CHECK_SAY ? ASK_OUTPUT : CHECK_SAY);
            return MRH_Module::FINISHED_APPEND;
            
        case ASK_OUTPUT:
            e_State = LISTEN_INPUT;
            return MRH_Module::FINISHED_APPEND;
            
        case LISTEN_INPUT:
            if (s_Input.size() == 0)
            {
                return MRH_Module::FINISHED_POP;
            }
            
            e_State = REPEAT_OUTPUT;
            return MRH_Module::FINISHED_APPEND;
            
        case REPEAT_OUTPUT:
            e_State = CLOSE_APP;
            return MRH_Module::FINISHED_APPEND;
            
        default:
            return MRH_Module::FINISHED_POP;
    }
}

std::shared_ptr<MRH_Module> MirrorSpeech::NextModule()
{
    switch (e_State)
    {
        case CHECK_LISTEN:
            return std::make_shared<MRH_CheckServiceModule>(MRH_CheckServiceModule::LISTEN,
                                                            MIRROR_SPEECH_SERVICE_TIMEOUT_MS,
                                                            b_ServiceAvailable);
        case CHECK_SAY:
            return std::make_shared<MRH_CheckServiceModule>(MRH_CheckServiceModule::SAY,
                                                            MIRROR_SPEECH_SERVICE_TIMEOUT_MS,
                                                            b_ServiceAvailable);
            
        case ASK_OUTPUT:
            try
            {
                return std::make_shared<MRH_SpeechOutputModule>(MRH_OutputGenerator(MIRROR_SPEECH_OUTPUT_DIR, 
                                                                                    MIRROR_SPEECH_OUTPUT_FILE).Generate(),
                                                                MIRROR_SPEECH_SPEECH_TIMEOUT_MS);
            }
            catch (MRH_VTException& e)
            {
                throw MRH_ModuleException("MirrorSpeech",
                                          "Failed to generate output: " + e.what2());
            }
            
        case LISTEN_INPUT:
            return std::make_shared<MRH_SpeechInputModule>(s_Input,
                                                           MIRROR_SPEECH_SPEECH_TIMEOUT_MS);
            
        case REPEAT_OUTPUT:
            return std::make_shared<MRH_SpeechOutputModule>(s_Input,
                                                            MIRROR_SPEECH_SPEECH_TIMEOUT_MS);
            
        default:
            throw MRH_ModuleException("MirrorSpeech",
                                      "No module to switch to!");
    }
}

//*************************************************************************************
// Getters
//*************************************************************************************

bool MirrorSpeech::CanHandleEvent(MRH_Uint32 u32_Type) noexcept
{
    return false;
}

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

// External
#include <libmrhvt/Output/MRH_OutputGenerator.h>
#include <libmrhvt/String/MRH_LocalisedPath.h>

// Project
#include "./MirrorSpeech.h"
#include "./SpeechInput.h"
#include "./SpeechOutput.h"

// Pre-defined
#ifndef MIRROR_SPEECH_OUTPUT_DIR
    #define MIRROR_SPEECH_OUTPUT_DIR "Output"
#endif
#ifndef MIRROR_SPEECH_OUTPUT_FILE
    #define MIRROR_SPEECH_OUTPUT_FILE "WhatInput.mrhog"
#endif


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

MirrorSpeech::MirrorSpeech() noexcept : MRH_Module("MirrorSpeech"),
                                        e_State(START),
                                        s_Input("")
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
            e_State = ASK_OUTPUT;
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
        case ASK_OUTPUT:
            try
            {
                return std::make_shared<SpeechOutput>(MRH_OutputGenerator(MRH_LocalisedPath::GetPath(MIRROR_SPEECH_OUTPUT_DIR, 
                                                                                                     MIRROR_SPEECH_OUTPUT_FILE)).Generate());
            }
            catch (MRH_VTException& e)
            {
                throw MRH_ModuleException("MirrorSpeech",
                                          "Failed to generate output: " + e.what2());
            }
            
        case LISTEN_INPUT:
            return std::make_shared<SpeechInput>(s_Input);
            
        case REPEAT_OUTPUT:
            return std::make_shared<SpeechOutput>(s_Input);
            
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

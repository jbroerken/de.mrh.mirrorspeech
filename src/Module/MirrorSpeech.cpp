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

// Project
#include "./MirrorSpeech.h"
#include "./CheckService.h"
#include "./SpeechOutput.h"
#include "./SpeechInput.h"

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
                                        s_Input(""),
                                        b_ServicesAvailable(false)
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
            e_State = CHECK_SERVICE;
            return MRH_Module::FINISHED_APPEND;
            
        case CHECK_SERVICE:
            if (b_ServicesAvailable == false)
            {
                return MRH_Module::FINISHED_POP;
            }
            
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
        case CHECK_SERVICE:
            return std::make_shared<CheckService>(b_ServicesAvailable);
            
        case ASK_OUTPUT:
            return std::make_shared<SpeechOutput>(MIRROR_SPEECH_OUTPUT_DIR,
                                                  MIRROR_SPEECH_OUTPUT_FILE);
            
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

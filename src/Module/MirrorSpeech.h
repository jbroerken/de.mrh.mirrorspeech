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

#ifndef MirrorSpeech_h
#define MirrorSpeech_h

// C / C++

// External
#include <libmrhab/Module/MRH_Module.h>

// Project


class MirrorSpeech : public MRH_Module
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    MirrorSpeech() noexcept;
    
    /**
     *  Default destructor.
     */
    
    ~MirrorSpeech() noexcept;
    
    //*************************************************************************************
    // Update
    //*************************************************************************************
    
    /**
     *  Hand a received event to the module.
     *
     *  \param p_Event The received event.
     */
    
    void HandleEvent(const MRH_Event* p_Event) noexcept override;
    
    /**
     *  Perform a module update.
     *
     *  \return The module update result.
     */
    
    MRH_Module::Result Update() override;
    
    /**
     *  Get the module to switch to.
     *
     *  \return The module switch information.
     */
    
    std::shared_ptr<MRH_Module> NextModule() override;
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Check if the module can handle a event.
     *
     *  \param u32_Type The type of the event to handle.
     *
     *  \return true if the event can be used, false if not.
     */
    
    bool CanHandleEvent(MRH_Uint32 u32_Type) noexcept override;
    
private:
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    enum State
    {
        START = 0,
        ASK_OUTPUT = 1,
        LISTEN_INPUT = 2,
        REPEAT_OUTPUT = 3,
        CLOSE_APP = 4,
        
        STATE_MAX = CLOSE_APP,
        
        STATE_COUNT = STATE_MAX + 1
    };
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    // Application state
    State e_State;

    // Module information
    std::string s_Input;
    
protected:

};

#endif /* MirrorSpeech_h */

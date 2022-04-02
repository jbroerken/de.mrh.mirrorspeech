/**
 *  MirrorSpeech.h
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
     *  Hand a recieved event to the module.
     *
     *  \param p_Event The recieved event.
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

/**
 *  RepeatAfterMe.h
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

#ifndef RepeatAfterMe_h
#define RepeatAfterMe_h

// C / C++

// External
#include <libmrhab/Module/MRH_Module.h>
#include <libmrhab/Module/Tools/MRH_ModuleTimer.h>
#include <libmrhvt/String/MRH_SpeechString.h>

// Project


class RepeatAfterMe : public MRH_Module,
                      private MRH_ModuleTimer
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    RepeatAfterMe();
    
    /**
     *  Default destructor.
     */
    
    ~RepeatAfterMe() noexcept;
    
    //*************************************************************************************
    // Update
    //*************************************************************************************
    
    /**
     *  Hand a recieved event to the module.
     *
     *  \param p_Event The recieved event.
     */
    
    void HandleEvent(const MRH_EVBase* p_Event) noexcept override;
    
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
        CHECK_SERVICE = 0,
        ASK_OUTPUT = 1,
        LISTEN_INPUT = 2,
        REPEAT_OUTPUT = 3,
        CLOSE_APP = 4,
        
        STATE_MAX = CLOSE_APP,
        
        STATE_COUNT = STATE_MAX + 1
    };
    
    //*************************************************************************************
    // State
    //*************************************************************************************
    
    /**
     *  Set the current state, resetting the timer.
     *
     *  \param e_State The next state to set.
     */
    
    void StateSet(State e_State) noexcept;
    
    /**
     *  Check service state.
     */
    
    void StateCheckService() noexcept;
    
    /**
     *  Send output created from a string.
     *
     *  \param s_String The string to send.
     */
    
    void StateSendOutput(std::string const& s_String) noexcept;
    
    /**
     *  Ask output state.
     */
    
    void StateAskOutput() noexcept;
    
    /**
     *  Repeat output state.
     */
    
    void StateRepeatOutput() noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    State e_State;
    
    int i_Service;
    MRH_SpeechString c_Input;
    MRH_Uint32 u32_OutputID;
    
protected:

};

#endif /* RepeatAfterMe_h */

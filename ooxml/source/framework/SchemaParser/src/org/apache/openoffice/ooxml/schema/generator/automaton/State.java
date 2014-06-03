/**************************************************************
*
* Licensed to the Apache Software Foundation (ASF) under one
* or more contributor license agreements.  See the NOTICE file
* distributed with this work for additional information
* regarding copyright ownership.  The ASF licenses this file
* to you under the Apache License, Version 2.0 (the
* "License"); you may not use this file except in compliance
* with the License.  You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an
* "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
* KIND, either express or implied.  See the License for the
* specific language governing permissions and limitations
* under the License.
*
*************************************************************/

package org.apache.openoffice.ooxml.schema.generator.automaton;

import java.util.Vector;

import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;

/** Each complex type is represented by a State object (primary state).
 *  For a validating parser additional states are created for sequences, choices, etc. (secondary states).
 *  Secondary states have the same basename as primary states and suffixes to make their names unique.
 *  Full names of states contain both the basename and the suffix.
 */
public class State
    implements Comparable<State>
{
    /** Create a new state from a basename and an optional suffix.
     *  When a state with the resulting name already exists in the state context
     *  then a RuntimeException is thrown.
     *  
     *  Call this method instead of GetState() when it is clear from the context
     *  that the state must not yet exist.
     */
    /** State objects can only be created via the GetState... methods.
     */
    State (
        final QualifiedName aBasename,
        final String sSuffix)
    {
        maBasename = aBasename;
        msSuffix = sSuffix;
        msFullname = GetStateName(aBasename, msSuffix);
        maTransitions = new Vector<>();
    }
    
    
    
    
    State Clone (final StateContext aContext)
    {
        return aContext.GetOrCreateState(maBasename, msSuffix);
    }
    
    
    
    
    static String GetStateName (
        final QualifiedName aBasename,
        final String sSuffix)
    {
        if (sSuffix == null)
            return aBasename.GetStateName();
        else
            return aBasename.GetStateName()+"_"+sSuffix;
    }
    

    
    
    public String GetFullname ()
    {
        return msFullname;
    }
    
    
    
    public QualifiedName GetBasename ()
    {
        return maBasename;
    }

    
    
    
    public String GetSuffix ()
    {
        return msSuffix;
    }

    
    
    
    public void AddTransition (final Transition aTransition)
    {
        assert(this == aTransition.GetStartState());
        maTransitions.add(aTransition);
    }
    
    
    
    
    public Iterable<Transition> GetTransitions()
    {
        return maTransitions;
    }

    
    
    
    /** Define how the optimizer will handle short circuits (epsilon paths)
     *  to the end state of an OOXML type.
     *  When there is an epsilon path from this state to aEndState then replace
     *  it with aReplacementState.
     *  It is an error to set two short circuits.  
     */
    public void SetShortCircuit (
        final State aEndState,
        final State aReplacementState)
    {
        assert(maShortCircuitEnd == null);
        maShortCircuitEnd = aEndState;
        maShortCircuitReplacement = aReplacementState;
    }


    
    
    public State GetShortCircuitEnd ()
    {
        return maShortCircuitEnd;
    }

    
    
    
    public State GetShortCircuitReplacement ()
    {
        return maShortCircuitReplacement;
    }
    
    
    
    
    /** The basename is the primary sort key.  The suffix is the secondary key.
     */
    @Override
    public int compareTo (final State aOther)
    {
        int nResult = maBasename.compareTo(aOther.maBasename);
        if (nResult == 0)
        {
            if (msSuffix==null && aOther.msSuffix==null)
                nResult = 0;
            else if (msSuffix!=null && aOther.msSuffix!=null)
                nResult = msSuffix.compareTo(aOther.msSuffix);
            else if (msSuffix==null)
                nResult = -1;
            else
                nResult = +1;
        }
        return nResult;
    }
    
    
    
    
    @Override
    public String toString ()
    {
        return msFullname;
    }
    
    
    
    
    private final QualifiedName maBasename;
    private final String msSuffix;
    private final String msFullname;
    private final Vector<Transition> maTransitions;
    /** Used in the optimization phase.
     *  When there is a way to reach the short circuit end only via epsilon transitions
     *  then replace it with the replacement state.
     *  This is to avoid jumps in sequences (or occurrences, etc.) when transitions become effectively optional.
     */
    private State maShortCircuitEnd;
    private State maShortCircuitReplacement;
}

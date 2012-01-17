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


/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AnimationSchemesPanel.hxx,v $
 * $Revision: 1.6 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SD_TASKPANE_CONTROLS_ANIMATION_SCHEMES_PANEL_HXX
#define SD_TASKPANE_CONTROLS_ANIMATION_SCHEMES_PANEL_HXX

#include "taskpane/SubToolPanel.hxx"

namespace sd {
class ViewShellBase;
}

namespace sd { namespace toolpanel {
class TreeNode;
} }

namespace sd { namespace toolpanel { namespace controls {

class AnimationSchemesPanel
    : public SubToolPanel
{
public:
    AnimationSchemesPanel (
        TreeNode* pParent,
        ViewShellBase& rBase);
    virtual ~AnimationSchemesPanel (void);

    virtual Size GetPreferredSize (void);
    virtual sal_Int32 GetPreferredWidth (sal_Int32 nHeigh);
    virtual sal_Int32 GetPreferredHeight (sal_Int32 nWidth);
    virtual ::Window* GetWindow (void);
    virtual bool IsResizable (void);
    virtual bool IsExpandable (void) const;

	using Window::GetWindow;

private:
    Size maPreferredSize;
    ::Window* mpWrappedControl;
};

} } } // end of namespace ::sd::toolpanel::controls

#endif
#ifndef SD_TASKPANE_CONTROLS_ANIMATION_SCHEMES_PANEL_HXX
#define SD_TASKPANE_CONTROLS_ANIMATION_SCHEMES_PANEL_HXX

#include "taskpane/SubToolPanel.hxx"

namespace sd {
class ViewShellBase;
}

namespace sd { namespace toolpanel {
class TreeNode;
} }

namespace sd { namespace toolpanel { namespace controls {

class AnimationSchemesPanel
    : public SubToolPanel
{
public:
    AnimationSchemesPanel (
        TreeNode* pParent,
        ViewShellBase& rBase);
    virtual ~AnimationSchemesPanel (void);

    virtual Size GetPreferredSize (void);
    virtual sal_Int32 GetPreferredWidth (sal_Int32 nHeigh);
    virtual sal_Int32 GetPreferredHeight (sal_Int32 nWidth);
    virtual ::Window* GetWindow (void);
    virtual bool IsResizable (void);
    virtual bool IsExpandable (void) const;

	using Window::GetWindow;

private:
    Size maPreferredSize;
    ::Window* mpWrappedControl;
};

} } } // end of namespace ::sd::toolpanel::controls

#endif

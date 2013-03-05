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

#ifndef SVX_PROPERTYPANEL_POSIZEPAGE_HXX
#define SVX_PROPERTYPANEL_POSIZEPAGE_HXX

#include <vcl/ctrl.hxx>
#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <boost/scoped_ptr.hpp>
#include <svx/rectenum.hxx>
#include <svl/poolitem.hxx>
#include <tools/fldunit.hxx>


class DialControl;
class SdrView;
class FixedText;
class MetricField;
class CheckBox;
class MetricBox;


namespace svx { namespace sidebar {

class TransformationPropertyPanel
:   public Control,
    public ::sfx2::sidebar::SidebarPanelBase::ContextChangeReceiverInterface,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    static TransformationPropertyPanel* Create(
        Window* pParent,
        const cssu::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);

    virtual void DataChanged(
        const DataChangedEvent& rEvent);

    virtual void HandleContextChange(
        const ::sfx2::sidebar::EnumContext aContext);

    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState);

    SfxBindings* GetBindings();
    void ShowMenu (void);

private:
    //Position
    ::boost::scoped_ptr< FixedText >        mpFtPosX;
    ::boost::scoped_ptr< MetricField >      mpMtrPosX;
    ::boost::scoped_ptr< FixedText >        mpFtPosY;
    ::boost::scoped_ptr< MetricField >      mpMtrPosY;

    // size
    ::boost::scoped_ptr< FixedText >        mpFtWidth;
    ::boost::scoped_ptr< MetricField >      mpMtrWidth;
    ::boost::scoped_ptr< FixedText >        mpFtHeight;
    ::boost::scoped_ptr< MetricField >      mpMtrHeight;
    ::boost::scoped_ptr< CheckBox >         mpCbxScale;

    //rotation
    ::boost::scoped_ptr< FixedText >        mpFtAngle;
    ::boost::scoped_ptr< MetricBox >        mpMtrAngle;

    //rotation control
    ::boost::scoped_ptr< DialControl >      mpDial;

    //flip
    ::boost::scoped_ptr< FixedText >        mpFtFlip;
    ::boost::scoped_ptr< Window >           mpFlipTbxBackground;
    ::boost::scoped_ptr< ToolBox >          mpFlipTbx;

    // Internal variables
    Rectangle                               maRect;
    const SdrView*                          mpView;
    sal_uInt32                              mlOldWidth;
    sal_uInt32                              mlOldHeight;
    RECT_POINT                              meRP;
    Point                                   maAnchorPos;    //anchor position
    long                                    mlRotX;
    long                                    mlRotY;
    Fraction                                maUIScale;
    SfxMapUnit                              mePoolUnit;
    FieldUnit                               meDlgUnit;

    // Controller Items
    ::sfx2::sidebar::ControllerItem         maTransfPosXControl;
    ::sfx2::sidebar::ControllerItem         maTransfPosYControl;
    ::sfx2::sidebar::ControllerItem         maTransfWidthControl;
    ::sfx2::sidebar::ControllerItem         maTransfHeightControl;
    ::sfx2::sidebar::ControllerItem         maTransAffineMatrix2DControl;

    ::sfx2::sidebar::ControllerItem         maSvxAngleControl;
    ::sfx2::sidebar::ControllerItem         maRotXControl;
    ::sfx2::sidebar::ControllerItem         maRotYControl;
    ::sfx2::sidebar::ControllerItem         maProPosControl;
    ::sfx2::sidebar::ControllerItem         maProSizeControl;
    ::sfx2::sidebar::ControllerItem         maAutoWidthControl;
    ::sfx2::sidebar::ControllerItem         maAutoHeightControl;
    ::sfx2::sidebar::ControllerItem         m_aMetricCtl;

    // images from ressource
    Image                                   maImgFlipHori;
    Image                                   maImgFlipVert;

    cssu::Reference< css::frame::XFrame >   mxFrame;
    ::sfx2::sidebar::EnumContext            maContext;
    SfxBindings*                            mpBindings;

    /// bitfield
    bool                                    mbMtrPosXMirror : 1;
    bool                                    mbSizeProtected : 1;
    bool                                    mbPositionProtected : 1;
    bool                                    mbAutoWidth : 1;
    bool                                    mbAutoHeight : 1;
    bool                                    mbAdjustEnabled : 1;
    bool                                    mbIsFlip : 1;
    bool                                    mbInDestructor : 1;

    DECL_LINK( ChangePosXHdl, void * );
    DECL_LINK( ChangePosYHdl, void * );
    DECL_LINK( ChangeWidthHdl, void * );
    DECL_LINK( ChangeHeightHdl, void * );
    DECL_LINK( ClickAutoHdl, void * );
    DECL_LINK( AngleModifiedHdl, void * );
    DECL_LINK( RotationHdl, void * );
    DECL_LINK( FlipHdl, ToolBox * );

    void SetupIcons(void);
    void Initialize();
    void executePosX();
    void executePosY();
    void executeSize();

    // constructor/destuctor
    TransformationPropertyPanel(
        Window* pParent,
        const cssu::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);
    virtual ~TransformationPropertyPanel();

    void MetricState( SfxItemState eState, const SfxPoolItem* pState );
    FieldUnit GetCurrentUnit( SfxItemState eState, const SfxPoolItem* pState );
    void DisableControls();
};


} } // end of namespace svx::sidebar



#endif // SVX_PROPERTYPANEL_POSIZEPAGE_HXX

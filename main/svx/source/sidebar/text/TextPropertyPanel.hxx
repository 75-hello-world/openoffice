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

#ifndef SVX_SIDEBAR_TEXT_PROPERTY_PAGE_HXX
#define SVX_SIDEBAR_TEXT_PROPERTY_PAGE_HXX

#include <vcl/ctrl.hxx>
#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/IContextChangeReceiver.hxx>

#include <svtools/ctrlbox.hxx>
#include <svx/tbxcolorupdate.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/fhgtitem.hxx>

#include <com/sun/star/ui/XUIElement.hpp>

#include <boost/scoped_ptr.hpp>


class FloatingWindow;
class ToolBox;

namespace svx { namespace sidebar {

class SvxSBFontNameBox;

class TextPropertyPanel
    : public Control,
      public ::sfx2::sidebar::IContextChangeReceiver,
      public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    static TextPropertyPanel* Create (
        Window* pParent,
        const cssu::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);

    virtual void DataChanged (const DataChangedEvent& rEvent);

    //	void SetDefaultUnderline(FontUnderline eUnderline);
    //	USHORT GetCurrColorType();

    //	void SetBackColor(Color aCol);
    //	void SetColor(Color aCol);
    //	void SetUnderline(FontUnderline	eUnderline);
    //	void SetSpacing(long nKern);

    /*
	SvxTextUnderlinePage* GetUnderlinePage();
	SfxPopupPanelWin* GetUnderlineFloatWin();

	SvxTextFontColorPage* GetFontColorPage();
	SfxPopupPanelWin* GetFontColorFloatWin();

	SvxTextSpacingPage* GetSpacingPage();
	SfxPopupPanelWin* GetSpacingFloatWin();
    */
    //	long GetSelFontSize();
    //	SfxPropertyPageController GetSpaceController();
//	ToolBox* GetSpacingTB();		//removed
    //	Color& GetUnderlineColor();  //
//	void FontListChanged();

    enum ColorType
    {
        FONT_COLOR = 1,
        BACK_COLOR = 2
    };

    virtual void HandleContextChange (
        const ::sfx2::sidebar::EnumContext aContext);

    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState);

    void ShowMenu (void);

private:
	//ui controls
    ::boost::scoped_ptr<SvxSBFontNameBox> mpFontNameBox;
	FontSizeBox maFontSizeBox;
	::boost::scoped_ptr<Window> mpToolBoxIncDecBackground;
	::boost::scoped_ptr<ToolBox> mpToolBoxIncDec;
	::boost::scoped_ptr<Window> mpToolBoxFontBackground;
	::boost::scoped_ptr<ToolBox> mpToolBoxFont;
	::boost::scoped_ptr<Window> mpToolBoxFontColorBackground;	
	::boost::scoped_ptr<ToolBox> mpToolBoxFontColor;	
	::boost::scoped_ptr<Window> mpToolBoxScriptBackground;
	::boost::scoped_ptr<ToolBox> mpToolBoxScript;
	::boost::scoped_ptr<Window> mpToolBoxScriptSwBackground;
	::boost::scoped_ptr<ToolBox> mpToolBoxScriptSw;
	::boost::scoped_ptr<Window> mpToolBoxSpacingBackground;
	::boost::scoped_ptr<ToolBox> mpToolBoxSpacing;
	::boost::scoped_ptr<Window> mpToolBoxHighlightBackground;
	::boost::scoped_ptr<ToolBox> mpToolBoxHighlight;
	::boost::scoped_ptr<ToolboxButtonColorUpdater> mpFontColorUpdater;
    ::boost::scoped_ptr<ToolboxButtonColorUpdater> mpHighlightUpdater;

	//control items
	::sfx2::sidebar::ControllerItem maFontNameControl;
	::sfx2::sidebar::ControllerItem maFontSizeControl;
	::sfx2::sidebar::ControllerItem maWeightControl;
	::sfx2::sidebar::ControllerItem maItalicControl;
	::sfx2::sidebar::ControllerItem maUnderlineControl;
	::sfx2::sidebar::ControllerItem maStrikeControl;
	::sfx2::sidebar::ControllerItem maShadowControl;
	::sfx2::sidebar::ControllerItem maFontColorControl;
	::sfx2::sidebar::ControllerItem maScriptControlSw;
	::sfx2::sidebar::ControllerItem maSuperScriptControl;
	::sfx2::sidebar::ControllerItem maSubScriptControl;
	::sfx2::sidebar::ControllerItem maSpacingControl;
	::sfx2::sidebar::ControllerItem maHighlightControl;
	::sfx2::sidebar::ControllerItem maSDFontGrow;
	::sfx2::sidebar::ControllerItem maSDFontShrink;

	//Images
	Image	maImgIncrease;
	Image	maImgDecrease;
	Image	maImgBold;
	Image	maImgItalic;
	Image	maImgUnderline;
	Image	maImgStrike;
	Image	maImgShadow;
	Image	maImgFontColor;
	Image	maImgSupScript;
	Image	maImgSubScript;
	Image   maImgHighlight;

	Image				maImgNormalIcon;

	Image	maImgIncreaseHigh;
	Image	maImgDecreaseHigh;
	Image	maImgBoldHigh;
	Image	maImgItalicHigh;
	Image	maImgUnderlineHigh;
	Image	maImgStrikeHigh;
	Image	maImgShadowHigh;
	Image	maImgFontColorHigh;
	Image	maImgSupScriptHigh;
	Image	maImgSubScriptHigh;
//	Image	maImgSpacingHigh;  // 
	Image   maImgHighlightHigh;

	FontWeight					meWeight;
	FontItalic					meItalic;
	FontUnderline				meUnderline;
	Color						meUnderlineColor;  //
	bool						mbShadow;
	FontStrikeout				meStrike;
	bool mbWeightAvailable;
	bool mbPostureAvailable;
	Color						maColor;
	bool mbColorAvailable;
	Color						maBackColor;
	bool mbBackColorAvailable;
	ColorType meColorType;
	SvxEscapement				meEscape;  //for sw
	bool						mbSuper;
	bool						mbSub;
	bool						mbKernAvailable;
	bool						mbKernLBAvailable;
	long						mlKerning;
	SvxFontHeightItem*			mpHeightItem;

	const FontList* mpFontList;
	bool mbMustDelete;
	bool mbFocusOnFontSizeCtrl;

    cssu::Reference<css::frame::XFrame> mxFrame;
    ::sfx2::sidebar::EnumContext maContext;
    SfxBindings* mpBindings;
    
    TextPropertyPanel (
        Window* pParent,
        const cssu::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);
    virtual ~TextPropertyPanel (void);

	void Initialize (void);
    void SetupIcons (void);
	void InitToolBoxFont();
	void InitToolBoxIncDec();
	void InitToolBoxFontColor();
	void InitToolBoxScript();
	void InitToolBoxHighlight();
	void InitToolBoxSpacing();

	void UpdateFontBold();
	void UpdateFontItalic();
	void UpdateFontUnderline();
	void UpdateFontStrikeOut();
	void UpdateFontShadowed();
	void UpdateFontScript();

    /*
	SfxPopupPanelWin*		mpFloatWinUnderline;
	class SvxTextUnderlinePage;
	SvxTextUnderlinePage*	mpPageUnderline; 

	SfxPopupPanelWin*		mpFloatWinFontColor;
	class SvxTextFontColorPage;
	SvxTextFontColorPage*	mpPageFontColor; 

	SfxPopupPanelWin*		mpFloatWinSpacing;
	class SvxTextSpacingPage;
	SvxTextSpacingPage*		mpPageSpacing;
    */
	DECL_LINK(FontSelHdl, FontNameBox *);
	DECL_LINK(FontSizeModifyHdl, FontSizeBox *);
	DECL_LINK(FontSizeSelHdl, FontSizeBox *);
	DECL_LINK(FontSizeLoseFocus, FontSizeBox *);
	DECL_LINK(ToolboxFontSelectHandler, ToolBox *);
	DECL_LINK(ToolboxIncDecSelectHdl, ToolBox *);
	DECL_LINK(ToolBoxUnderlineClickHdl, ToolBox* );
	DECL_LINK(ImplPopupModeEndHdl, FloatingWindow* );
	DECL_LINK(ImplSpacingPopupModeEndHdl, FloatingWindow* );  //
	DECL_LINK(ToolBoxFontColorDropHdl, ToolBox *);
	DECL_LINK(ToolBoxSwScriptSelectHdl, ToolBox *);
	DECL_LINK(ToolBoxScriptSelectHdl, ToolBox *);
	DECL_LINK(SpacingClickHdl, ToolBox*);
	DECL_LINK(ToolBoxHighlightDropHdl, ToolBox *);

	void TextStyleChanged();

    Image GetIcon (const ::rtl::OUString& rsURL);
};

} } // end of namespace ::svx::sidebar

#endif

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

#include "precompiled_svx.hxx"
#include "TextUnderlineControl.hxx"
#include "TextPropertyPanel.hrc"
#include <sfx2/sidebar/propertypanel.hrc>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <unotools/viewoptions.hxx>
#include <editeng/kernitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <editeng/udlnitem.hxx>

namespace svx { namespace sidebar {

TextUnderlineControl::TextUnderlineControl (
    Window* pParent,
    svx::sidebar::TextPropertyPanel& rPanel,
    SfxBindings* pBindings)
:	svx::sidebar::PopupControl( pParent,SVX_RES(RID_POPUPPANEL_TEXTPAGE_UNDERLINE))
,	mrTextPropertyPanel(rPanel)
,	mpBindings(pBindings)
,	maVSUnderline( this, SVX_RES(VS_UNDERLINE))
,	maPBOptions	(this, SVX_RES(PB_OPTIONS) )

,	maIMGSingle		(SVX_RES(IMG_SINGLE))
,	maIMGDouble		(SVX_RES(IMG_DOUBLE))
,	maIMGBold		(SVX_RES(IMG_BOLD2))
,	maIMGDot		(SVX_RES(IMG_DOT))
,	maIMGDotBold	(SVX_RES(IMG_DOT_BOLD))
,	maIMGDash		(SVX_RES(IMG_DASH))
,	maIMGDashLong	(SVX_RES(IMG_DASH_LONG))
,	maIMGDashDot	(SVX_RES(IMG_DASH_DOT))
,	maIMGDashDotDot	(SVX_RES(IMG_DASH_DOT_DOT))
,	maIMGWave		(SVX_RES(IMG_WAVE))

// high contrast
,	maIMGSingleH	(SVX_RES(IMG_SINGLE_H))
,	maIMGDoubleH	(SVX_RES(IMG_DOUBLE_H))
,	maIMGBoldH		(SVX_RES(IMG_BOLD2_H))
,	maIMGDotH		(SVX_RES(IMG_DOT_H))
,	maIMGDotBoldH	(SVX_RES(IMG_DOT_BOLD_H))
,	maIMGDashH		(SVX_RES(IMG_DASH_H))
,	maIMGDashLongH	(SVX_RES(IMG_DASH_LONG_H))
,	maIMGDashDotH	(SVX_RES(IMG_DASH_DOT_H))
,	maIMGDashDotDotH(SVX_RES(IMG_DASH_DOT_DOT_H))
,	maIMGWaveH		(SVX_RES(IMG_WAVE_H))

,	maIMGSingleSel		(SVX_RES(IMG_SINGLE_SEL))
,	maIMGDoubleSel		(SVX_RES(IMG_DOUBLE_SEL))
,	maIMGBoldSel		(SVX_RES(IMG_BOLD2_SEL))
,	maIMGDotSel			(SVX_RES(IMG_DOT_SEL))
,	maIMGDotBoldSel		(SVX_RES(IMG_DOT_BOLD_SEL))
,	maIMGDashSel		(SVX_RES(IMG_DASH_SEL))
,	maIMGDashLongSel	(SVX_RES(IMG_DASH_LONG_SEL))
,	maIMGDashDotSel		(SVX_RES(IMG_DASH_DOT_SEL))
,	maIMGDashDotDotSel	(SVX_RES(IMG_DASH_DOT_DOT_SEL))
,	maIMGWaveSel		(SVX_RES(IMG_WAVE_SEL))

{
	initial();
	FreeResource();
}

void TextUnderlineControl::initial()
{
	/*maPBOptions.SetDefBkColor(GetSettings().GetStyleSettings().GetHighContrastMode()?
		GetSettings().GetStyleSettings().GetMenuColor():
		sfx2::sidebar::Theme::GetColor( sfx2::sidebar::Theme::Paint_DropDownBackground ));//Color(244,245,249)//for high contract
	maPBOptions.SetHoverBkColor(GetSettings().GetStyleSettings().GetHighContrastMode()?
		GetSettings().GetStyleSettings().GetMenuColor():
		sfx2::sidebar::Theme::GetColor( sfx2::sidebar::Theme::Paint_PanelBackground ) );//Color( 93, 120, 163 )
	maPBOptions.SetHoverTxtColor( sfx2::sidebar::Theme::GetColor( sfx2::sidebar::Theme::Color_PanelTitleFont ) );//Color( 255, 255, 255 )
	maPBOptions.SetIcoPosX( 2);*/
	maVSUnderline.SetColor(GetSettings().GetStyleSettings().GetHighContrastMode()?
		GetSettings().GetStyleSettings().GetMenuColor():
		sfx2::sidebar::Theme::GetColor( sfx2::sidebar::Theme::Paint_PanelBackground ));
	maVSUnderline.SetBackground(GetSettings().GetStyleSettings().GetHighContrastMode()?
		GetSettings().GetStyleSettings().GetMenuColor():
		sfx2::sidebar::Theme::GetColor( sfx2::sidebar::Theme::Paint_PanelBackground ));

	Link aLink = LINK( this, TextUnderlineControl, PBClickHdl ) ;
	maPBOptions.SetClickHdl(aLink);

	maVSUnderline.SetStyle( maVSUnderline.GetStyle()| WB_3DLOOK |  WB_NO_DIRECTSELECT  );// WB_NAMEFIELD | WB_ITEMBORDER |WB_DOUBLEBORDER | WB_NONEFIELD |

	maVSUnderline.InsertItem(1, maIMGSingle ,String(SVX_RES(STR_SINGLE)));
	maVSUnderline.SetItemData(1, (void*)(sal_uInt64)UNDERLINE_SINGLE);

	maVSUnderline.InsertItem(2, maIMGDouble ,String(SVX_RES(STR_DOUBLE)));
	maVSUnderline.SetItemData(2, (void*)(sal_uInt64)UNDERLINE_DOUBLE);

	maVSUnderline.InsertItem(3, maIMGBold, String(SVX_RES(STR_BOLD)));
	maVSUnderline.SetItemData(3,(void*)(sal_uInt64)UNDERLINE_BOLD);

	maVSUnderline.InsertItem(4, maIMGDot, String(SVX_RES(STR_DOT)));
	maVSUnderline.SetItemData(4,(void*)(sal_uInt64)UNDERLINE_DOTTED);

	maVSUnderline.InsertItem(5, maIMGDotBold, String(SVX_RES(STR_DOT_BOLD)));
	maVSUnderline.SetItemData(5,(void*)(sal_uInt64)UNDERLINE_BOLDDOTTED);

	maVSUnderline.InsertItem(6, maIMGDash, String(SVX_RES(STR_DASH)));
	maVSUnderline.SetItemData(6,(void*)(sal_uInt64)UNDERLINE_DASH);

	maVSUnderline.InsertItem(7, maIMGDashLong,String(SVX_RES(STR_DASH_LONG)));
	maVSUnderline.SetItemData(7,(void*)(sal_uInt64)UNDERLINE_LONGDASH);

	maVSUnderline.InsertItem(8, maIMGDashDot, String(SVX_RES(STR_DASH_DOT)));
	maVSUnderline.SetItemData(8,(void*)(sal_uInt64)UNDERLINE_DASHDOT);

	maVSUnderline.InsertItem(9, maIMGDashDotDot, String(SVX_RES(STR_DASH_DOT_DOT)));
	maVSUnderline.SetItemData(9,(void*)(sal_uInt64)UNDERLINE_DASHDOTDOT);

	maVSUnderline.InsertItem(10, maIMGWave, String(SVX_RES(STR_WAVE)));
	maVSUnderline.SetItemData(10,(void*)(sal_uInt64)UNDERLINE_WAVE);

	maVSUnderline.SetColCount( 1 );
	aLink =  LINK( this, TextUnderlineControl, VSSelectHdl ) ;
    maVSUnderline.SetSelectHdl(aLink);

	maVSUnderline.StartSelection();
	maVSUnderline.Show();
}

void TextUnderlineControl::GetFocus()
{
	maVSUnderline.GrabFocus();
}
void TextUnderlineControl::Rearrange(FontUnderline eLine)
{
	// high contrast
	maVSUnderline.SetItemImage(1,  GetDisplayBackground().GetColor().IsDark()? maIMGSingleH :maIMGSingle);
	maVSUnderline.SetItemImage(2,  GetDisplayBackground().GetColor().IsDark()? maIMGDoubleH : maIMGDouble );
	maVSUnderline.SetItemImage(3, GetDisplayBackground().GetColor().IsDark()? maIMGBoldH : maIMGBold);
	maVSUnderline.SetItemImage(4, GetDisplayBackground().GetColor().IsDark()? maIMGDotH : maIMGDot);
	maVSUnderline.SetItemImage(5, GetDisplayBackground().GetColor().IsDark()? maIMGDotBoldH :maIMGDotBold);
	maVSUnderline.SetItemImage(6,  GetDisplayBackground().GetColor().IsDark()? maIMGDashH :maIMGDash);
	maVSUnderline.SetItemImage(7, GetDisplayBackground().GetColor().IsDark()? maIMGDashLongH : maIMGDashLong);
	maVSUnderline.SetItemImage(8, GetDisplayBackground().GetColor().IsDark()? maIMGDashDotH : maIMGDashDot); 
	maVSUnderline.SetItemImage(9, GetDisplayBackground().GetColor().IsDark()? maIMGDashDotDotH : maIMGDashDotDot);
	maVSUnderline.SetItemImage(10, GetDisplayBackground().GetColor().IsDark()? maIMGWaveH : maIMGWave);
//	maVSUnderline.SelectItem(0);		//delete 
//	maVSUnderline.SetNoSelection();

	switch(eLine)
	{
	case UNDERLINE_SINGLE:
		maVSUnderline.SetItemImage(1, maIMGSingleSel);
		maVSUnderline.SelectItem(1);	//add 
		maVSUnderline.GrabFocus();
		break;
	case UNDERLINE_DOUBLE:
		maVSUnderline.SetItemImage(2, maIMGDoubleSel);
		maVSUnderline.SelectItem(2);	//add 
		maVSUnderline.GrabFocus();
		break;
	case UNDERLINE_BOLD:
		maVSUnderline.SetItemImage(3, maIMGBoldSel);
		maVSUnderline.SelectItem(3);	//add 
		maVSUnderline.GrabFocus();
		break;	
	case UNDERLINE_DOTTED:
		maVSUnderline.SetItemImage(4, maIMGDotSel);
		maVSUnderline.SelectItem(4);	//add 
		maVSUnderline.GrabFocus();
		break;
	case UNDERLINE_BOLDDOTTED:
		maVSUnderline.SetItemImage(5, maIMGDotBoldSel);
		maVSUnderline.SelectItem(5);	//add 
		maVSUnderline.GrabFocus();
		break;
	case UNDERLINE_DASH:
		maVSUnderline.SetItemImage(6, maIMGDashSel);
		maVSUnderline.SelectItem(6);	//add 
		maVSUnderline.GrabFocus();
		break;
	case UNDERLINE_LONGDASH:
		maVSUnderline.SetItemImage(7, maIMGDashLongSel);
		maVSUnderline.SelectItem(7);	//add 
		maVSUnderline.GrabFocus();
		break;
	case UNDERLINE_DASHDOT:
		maVSUnderline.SetItemImage(8, maIMGDashDotSel);
		maVSUnderline.SelectItem(8);	//add 
		maVSUnderline.GrabFocus();
		break;
	case UNDERLINE_DASHDOTDOT:
		maVSUnderline.SetItemImage(9, maIMGDashDotDotSel);
		maVSUnderline.SelectItem(9);	//add 
		maVSUnderline.GrabFocus();
		break;
	case UNDERLINE_WAVE:
		maVSUnderline.SetItemImage(10, maIMGWaveSel);
		maVSUnderline.SelectItem(10);	//add 
		maVSUnderline.GrabFocus();
		break;
	case UNDERLINE_NONE:
	default:
		maVSUnderline.SelectItem(1);
		maVSUnderline.SetNoSelection();//add 
		maPBOptions.GrabFocus();
	}
	maVSUnderline.StartSelection();
	//removed 
	//if(mpPage->meContextType == PROPERTY_CONTEXT_SC_CELL)
	//	maPBOptions.Disable();
	//else
	//	maPBOptions.Enable();
	//removed end
}
ValueSet& TextUnderlineControl::GetValueSet()
{
	return maVSUnderline;
}
Control& TextUnderlineControl::GetPB()
{
	return maPBOptions;
}
IMPL_LINK(TextUnderlineControl, VSSelectHdl, void *, pControl)
{
	if(pControl == &maVSUnderline)  
	{
		sal_uInt16 iPos = maVSUnderline.GetSelectItemId();
		FontUnderline eUnderline = (FontUnderline)(sal_uInt64)maVSUnderline.GetItemData( iPos );

		//<<modified 
		//SvxTextLineItem aLineItem(eUnderline, SID_ATTR_CHAR_UNDERLINE);
		SvxUnderlineItem aLineItem(eUnderline, SID_ATTR_CHAR_UNDERLINE);
		//modify end>>

		//<<add , this line of code will keep the new underline use pre-color
		aLineItem.SetColor(mrTextPropertyPanel.GetUnderlineColor());   
		//add end>>
		mpBindings->GetDispatcher()->Execute(SID_ATTR_CHAR_UNDERLINE, SFX_CALLMODE_RECORD, &aLineItem, 0L);

		//add , for the popup page not update immediately
		mrTextPropertyPanel.SetUnderline(eUnderline);
		//add end
		//mrTextPropertyPanel.SetDefaultUnderline(eUnderline);

		mrTextPropertyPanel.EndUnderlinePopupMode();
	}
	return( 0L );
}

IMPL_LINK(TextUnderlineControl, PBClickHdl, PushButton *, pPBtn)
{
	if(pPBtn == &maPBOptions)  
	{
		if (mpBindings)
		{
			SfxDispatcher* pDisp = mpBindings->GetDispatcher();
			pDisp->Execute( SID_CHAR_DLG_EFFECT, SFX_CALLMODE_ASYNCHRON );
		}
		//add 
		mrTextPropertyPanel.EndUnderlinePopupMode();
		//add end
	}
	return 0;
}

}}
